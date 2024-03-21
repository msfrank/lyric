
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/call_cell.h>
#include <lyric_runtime/stackful_coroutine.h>
#include <lyric_runtime/subroutine_manager.h>
#include <tempo_utils/big_endian.h>

lyric_runtime::SubroutineManager::SubroutineManager(SegmentManager *segmentManager)
    : m_segmentManager(segmentManager)
{
    TU_ASSERT (m_segmentManager != nullptr);
}

bool
lyric_runtime::SubroutineManager::callProc(
    tu_uint32 callIndex,
    BytecodeSegment *segment,
    tu_uint32 procOffset,
    std::vector<DataCell> &args,
    StackfulCoroutine *currentCoro,
    tempo_utils::Status &status)
{
    TU_ASSERT (segment != nullptr);
    TU_ASSERT (currentCoro != nullptr);

    // proc offset must be within the segment bytecode
    auto *bytecodeData = segment->getBytecodeData();
    auto bytecodeSize = segment->getBytecodeSize();
    if (bytecodeSize <= procOffset) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid proc offset");
        return false;
    }

    const uint8_t *header = bytecodeData + procOffset;
    const BytecodeSegment *returnSP = currentCoro->peekSP();
    const lyric_object::BytecodeIterator returnIP = currentCoro->peekIP();
    auto stackGuard = currentCoro->dataStackSize();

    // read the call proc header
    auto procSize = tempo_utils::read_u32_and_advance(header);
    auto numArguments = tempo_utils::read_u16_and_advance(header);
    auto numLocals = tempo_utils::read_u16_and_advance(header);
    auto numLexicals = tempo_utils::read_u16_and_advance(header);
    auto codeSize = procSize - 6;

    // maximum number of args is 2^16
    if (std::numeric_limits<uint16_t>::max() <= args.size()) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "too many arguments");
        return false;
    }

    // all required args must be present
    if (args.size() < numArguments) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "not enough arguments");
        return false;
    }

    auto numRest = static_cast<uint16_t>(args.size()) - numArguments;

    // construct the activation call frame
    CallCell frame(callIndex, segment->getSegmentIndex(), procOffset, returnSP->getSegmentIndex(),
        returnIP, stackGuard, numArguments, numRest, numLocals, numLexicals, args);

    // import each lexical from the latest activation and push it onto the stack
    for (uint16_t i = 0; i < numLexicals; i++) {

        // read the call proc lexical
        auto activationCall = tempo_utils::read_u32_and_advance(header);
        auto targetOffset = tempo_utils::read_u32_and_advance(header);
        auto lexicalTarget = tempo_utils::read_u8_and_advance(header);
        codeSize -= 9;

        bool found = false;
        for (auto iterator = currentCoro->callsBegin(); iterator != currentCoro->callsEnd(); iterator++) {
            const auto &ancestor = *iterator;

            if (ancestor.getCallSegment() != segment->getSegmentIndex())    // lexical must exist in callee segment
                continue;
            if (ancestor.getCallIndex() != activationCall)                  // frame does not match activation call
                continue;
            switch (lexicalTarget) {
                case lyric_object::LEXICAL_ARGUMENT:
                    frame.setLexical(i, ancestor.getArgument(targetOffset));
                    break;
                case lyric_object::LEXICAL_LOCAL:
                    frame.setLexical(i, ancestor.getLocal(targetOffset));
                    break;
                default:
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid lexical target");
                    return false;
            }
            found = true;                       // we found the lexical, break loop early
            break;
        }
        if (!found) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "missing lexical");
            return false;
        }
    }

    lyric_object::BytecodeIterator ip(header, codeSize);
    currentCoro->pushCall(frame, ip, segment);               // push the activation onto the call stack
    TU_LOG_V << "moved ip to " << ip;

    return true;
}

bool
lyric_runtime::SubroutineManager::callStatic(
    tu_uint32 address,
    std::vector<DataCell> &args,
    StackfulCoroutine *currentCoro,
    tempo_utils::Status &status)
{
    TU_ASSERT (currentCoro != nullptr);

    BytecodeSegment *segment = nullptr;
    tu_uint32 callIndex;
    tu_uint32 procOffset;
    lyric_object::CallWalker call;

    auto *sp = currentCoro->peekSP();

    // resolve the segment and call proc
    if (lyric_object::IS_NEAR(address)) {
        segment = sp;
        auto object = segment->getObject().getObject();
        call = object.getCall(address);
        if (!call.isValid()) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "missing call descriptor");
            return false;
        }
        callIndex = address;
        procOffset = call.getProcOffset();
    } else {
        const auto *linkage = m_segmentManager->resolveLink(sp, address, status);
        if (!linkage || linkage->linkage != lyric_object::LinkageSection::Call) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid call linkage");
            return false;
        }
        segment = m_segmentManager->getSegment(linkage->assembly);
        auto object = segment->getObject().getObject();
        call = object.getCall(linkage->value);
        if (!call.isValid()) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "missing call descriptor");
            return false;
        }
        callIndex = linkage->value;
        procOffset = call.getProcOffset();
    }

    // validate call descriptor
    if (call.getMode() == lyric_object::CallMode::Constructor) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid call flags");
        return false;
    }
    if (call.isBound()) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid call flags");
        return false;
    }

    // proc offset must be within the segment bytecode
    auto *bytecodeData = segment->getBytecodeData();
    auto bytecodeSize = segment->getBytecodeSize();
    if (bytecodeSize <= procOffset) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid proc offset");
        return false;
    }

    const uint8_t *header = bytecodeData + procOffset;
    const BytecodeSegment *returnSP = sp;
    const lyric_object::BytecodeIterator returnIP = currentCoro->peekIP();
    auto stackGuard = currentCoro->dataStackSize();

    // read the call proc header
    auto procSize = tempo_utils::read_u32_and_advance(header);
    auto numArguments = tempo_utils::read_u16_and_advance(header);
    auto numLocals = tempo_utils::read_u16_and_advance(header);
    auto numLexicals = tempo_utils::read_u16_and_advance(header);
    auto codeSize = procSize - 6;

    // maximum number of args is 2^16
    if (std::numeric_limits<uint16_t>::max() <= args.size()) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "too many arguments");
        return false;
    }

    // all required args must be present
    if (args.size() < numArguments) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "not enough arguments");
        return false;
    }

    auto numRest = static_cast<uint16_t>(args.size()) - numArguments;

    // construct the activation call frame
    CallCell frame(callIndex, segment->getSegmentIndex(), procOffset, returnSP->getSegmentIndex(),
        returnIP, stackGuard, numArguments, numRest, numLocals, numLexicals, args);

    // import each lexical from the latest activation and push it onto the stack
    for (uint16_t i = 0; i < numLexicals; i++) {

        // read the call proc lexical
        auto activationCall = tempo_utils::read_u32_and_advance(header);
        auto targetOffset = tempo_utils::read_u32_and_advance(header);
        auto lexicalTarget = tempo_utils::read_u8_and_advance(header);
        codeSize -= 9;

        bool found = false;
        for (auto iterator = currentCoro->callsBegin(); iterator != currentCoro->callsEnd(); iterator++) {
            const auto &ancestor = *iterator;

            if (ancestor.getCallSegment() != segment->getSegmentIndex())    // lexical must exist in callee segment
                continue;
            if (ancestor.getCallIndex() != activationCall)                  // frame does not match activation call
                continue;
            switch (lexicalTarget) {
                case lyric_object::LEXICAL_ARGUMENT:
                    frame.setLexical(i, ancestor.getArgument(targetOffset));
                    break;
                case lyric_object::LEXICAL_LOCAL:
                    frame.setLexical(i, ancestor.getLocal(targetOffset));
                    break;
                default:
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid lexical target");
                    return false;
            }
            found = true;                       // we found the lexical, break loop early
            break;
        }
        if (!found) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "missing lexical");
            return false;
        }
    }

    lyric_object::BytecodeIterator ip(header, codeSize);
    currentCoro->pushCall(frame, ip, segment);               // push the activation onto the call stack
    TU_LOG_V << "moved ip to " << ip;

    return true;
}

bool
lyric_runtime::SubroutineManager::callVirtual(
    BaseRef *receiver,
    tu_uint32 address,
    std::vector<DataCell> &args,
    StackfulCoroutine *currentCoro,
    tempo_utils::Status &status)
{
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (currentCoro != nullptr);

    auto *sp = currentCoro->peekSP();

    // resolve address to a descriptor
    auto descriptor = m_segmentManager->resolveDescriptor(
        sp, lyric_object::LinkageSection::Call, address, status);
    if (!descriptor.isValid())
        return false;

    // resolve the descriptor  to a vtable entry
    const auto *vtable = receiver->getVirtualTable();
    TU_ASSERT (vtable != nullptr);
    const auto *method = vtable->getMethod(descriptor);
    if (method == nullptr) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "missing virtual method");
        return false;
    }

    auto *segment = method->getSegment();
    const auto callIndex = method->getCallIndex();
    const auto procOffset = method->getProcOffset();

    // proc offset must be within the segment bytecode
    auto *bytecodeData = segment->getBytecodeData();
    auto bytecodeSize = segment->getBytecodeSize();
    if (bytecodeSize <= procOffset) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid proc offset");
        return false;
    }

    const uint8_t *header = bytecodeData + procOffset;
    const BytecodeSegment *returnSP = sp;
    const lyric_object::BytecodeIterator returnIP = currentCoro->peekIP();
    auto stackGuard = currentCoro->dataStackSize();

    // read the call proc header
    auto procSize = tempo_utils::read_u32_and_advance(header);
    auto numArguments = tempo_utils::read_u16_and_advance(header);
    auto numLocals = tempo_utils::read_u16_and_advance(header);
    auto numLexicals = tempo_utils::read_u16_and_advance(header);
    auto codeSize = procSize - 6;

    // maximum number of args is 2^16
    if (std::numeric_limits<uint16_t>::max() <= args.size()) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "too many arguments");
        return false;
    }

    // all required args must be present
    if (args.size() < numArguments) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "not enough arguments");
        return false;
    }

    auto numRest = static_cast<uint16_t>(args.size()) - numArguments;

    // construct the activation call frame
    CallCell frame(callIndex, segment->getSegmentIndex(), procOffset, returnSP->getSegmentIndex(),
        returnIP, stackGuard, numArguments, numRest, numLocals, numLexicals, args, DataCell::forRef(receiver));

    // import each lexical from the latest activation and push it onto the stack
    for (uint16_t i = 0; i < numLexicals; i++) {

        // read the call proc lexical
        auto activationCall = tempo_utils::read_u32_and_advance(header);
        auto targetOffset = tempo_utils::read_u32_and_advance(header);
        auto lexicalTarget = tempo_utils::read_u8_and_advance(header);
        codeSize -= 9;

        bool found = false;
        for (auto iterator = currentCoro->callsBegin(); iterator != currentCoro->callsEnd(); iterator++) {
            const auto &ancestor = *iterator;

            if (ancestor.getCallSegment() != segment->getSegmentIndex())    // lexical must exist in callee segment
                continue;
            if (ancestor.getCallIndex() != activationCall)                  // frame does not match activation call
                continue;
            switch (lexicalTarget) {
                case lyric_object::LEXICAL_ARGUMENT:
                    frame.setLexical(i, ancestor.getArgument(targetOffset));
                    break;
                case lyric_object::LEXICAL_LOCAL:
                    frame.setLexical(i, ancestor.getLocal(targetOffset));
                    break;
                default:
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid lexical target");
                    return false;
            }
            found = true;                       // we found the lexical, break loop early
            break;
        }
        if (!found) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "missing lexical");
            return false;
        }
    }

    lyric_object::BytecodeIterator ip(header, codeSize);
    currentCoro->pushCall(frame, ip, segment);               // push the activation onto the call stack
    TU_LOG_V << "moved ip to " << ip;

    return true;
}

bool
lyric_runtime::SubroutineManager::callConcept(
    BaseRef *receiver,
    const DataCell &conceptDescriptor,
    tu_uint32 actionAddress,
    std::vector<DataCell> &args,
    StackfulCoroutine *currentCoro,
    tempo_utils::Status &status)
{
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (currentCoro != nullptr);

    auto *sp = currentCoro->peekSP();

    const auto *vtable = receiver->getVirtualTable();
    if (vtable == nullptr)
        return false;

    // resolve address to a descriptor
    auto actionDescriptor = m_segmentManager->resolveDescriptor(
        sp, lyric_object::LinkageSection::Action, actionAddress, status);
    if (!actionDescriptor.isValid())
        return false;

    // resolve the descriptor to a vtable entry
    const auto *method = vtable->getExtension(conceptDescriptor, actionDescriptor);
    if (method == nullptr) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "missing extension method");
        return false;
    }

    auto *segment = method->getSegment();
    const auto callIndex = method->getCallIndex();
    const auto procOffset = method->getProcOffset();

    // proc offset must be within the segment bytecode
    auto *bytecodeData = segment->getBytecodeData();
    auto bytecodeSize = segment->getBytecodeSize();
    if (bytecodeSize <= procOffset) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid proc offset");
        return false;
    }

    const uint8_t *header = bytecodeData + procOffset;
    const BytecodeSegment *returnSP = sp;
    const lyric_object::BytecodeIterator returnIP = currentCoro->peekIP();
    auto stackGuard = currentCoro->dataStackSize();

    // read the call proc header
    auto procSize = tempo_utils::read_u32_and_advance(header);
    auto numArguments = tempo_utils::read_u16_and_advance(header);
    auto numLocals = tempo_utils::read_u16_and_advance(header);
    auto numLexicals = tempo_utils::read_u16_and_advance(header);
    auto codeSize = procSize - 6;

    // maximum number of args is 2^16
    if (std::numeric_limits<uint16_t>::max() <= args.size()) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "too many arguments");
        return false;
    }

    // all required args must be present
    if (args.size() < numArguments) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "not enough arguments");
        return false;
    }

    auto numRest = static_cast<uint16_t>(args.size()) - numArguments;

    // construct the activation call frame
    CallCell frame(callIndex, segment->getSegmentIndex(), procOffset, returnSP->getSegmentIndex(),
        returnIP, stackGuard, numArguments, numRest, numLocals, numLexicals, args, DataCell::forRef(receiver));

    // import each lexical from the latest activation and push it onto the stack
    for (uint16_t i = 0; i < numLexicals; i++) {

        // read the call proc lexical
        auto activationCall = tempo_utils::read_u32_and_advance(header);
        auto targetOffset = tempo_utils::read_u32_and_advance(header);
        auto lexicalTarget = tempo_utils::read_u8_and_advance(header);
        codeSize -= 9;

        bool found = false;
        for (auto iterator = currentCoro->callsBegin(); iterator != currentCoro->callsEnd(); iterator++) {
            const auto &ancestor = *iterator;

            if (ancestor.getCallSegment() != segment->getSegmentIndex())    // lexical must exist in callee segment
                continue;
            if (ancestor.getCallIndex() != activationCall)                  // frame does not match activation call
                continue;
            switch (lexicalTarget) {
                case lyric_object::LEXICAL_ARGUMENT:
                    frame.setLexical(i, ancestor.getArgument(targetOffset));
                    break;
                case lyric_object::LEXICAL_LOCAL:
                    frame.setLexical(i, ancestor.getLocal(targetOffset));
                    break;
                default:
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid lexical target");
                    return false;
            }
            found = true;                       // we found the lexical, break loop early
            break;
        }
        if (!found) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "missing lexical");
            return false;
        }
    }

    lyric_object::BytecodeIterator ip(header, codeSize);
    currentCoro->pushCall(frame, ip, segment);               // push the activation onto the call stack
    TU_LOG_V << "moved ip to " << ip;

    return true;
}

bool
lyric_runtime::SubroutineManager::initStatic(
    tu_uint32 address,
    StackfulCoroutine *currentCoro,
    tempo_utils::Status &status)
{
    BytecodeSegment *segment = nullptr;
    lyric_object::ObjectWalker object;
    lyric_object::StaticWalker static_;

    auto *sp = currentCoro->peekSP();

    if (lyric_object::IS_NEAR(address)) {
        segment = sp;
        object = sp->getObject().getObject();
        static_ = object.getStatic(address);
    } else {
        const auto *linkage = m_segmentManager->resolveLink(sp, address, status);
        if (linkage == nullptr)
            return false;                   // failed to resolve dynamic link
        if (linkage->linkage != lyric_object::LinkageSection::Static) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid static linkage");
            return false;
        }
        segment = m_segmentManager->getSegment(linkage->assembly);
        object = segment->getObject().getObject();
        static_ = object.getStatic(linkage->value);
    }

    if (!static_.isValid()) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "missing static symbol");
        return false;
    }

    // FIXME: we should be able to support a far initializer
    auto initializer = static_.getNearInitializer();
    if (!initializer.isValid()) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid static init");
        return false;
    }

    auto callIndex = initializer.getDescriptorOffset();
    auto procOffset = initializer.getProcOffset();

    std::vector<DataCell> args;
    return callProc(callIndex, segment, procOffset, args, currentCoro, status);
}

bool
lyric_runtime::SubroutineManager::returnToCaller(
    StackfulCoroutine *currentCoro,
    tempo_utils::Status &status)
{
    TU_ASSERT (currentCoro != nullptr);

    // if the call stack is empty then set status and return false
    if (currentCoro->callStackSize() == 0) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "call stack is empty");
        return false;
    }

    // otherwise pop the top of the call stack and restore the *SP and *IP registers from the activation frame
    auto frame = currentCoro->popCall();
    auto ip = frame.getReturnIP();
    auto sp = m_segmentManager->getSegment(frame.getReturnSegment());
    currentCoro->transferControl(ip, sp);

    // drop leftover temporaries from the stack, but preserve the top most item as the return value
    if (currentCoro->dataStackSize() > frame.getStackGuard()) {
        auto returnValue = currentCoro->popData();
        currentCoro->resizeDataStack(frame.getStackGuard());
        currentCoro->pushData(returnValue);
    }

    // return false if IP is not valid, which causes the program to halt
    if (!ip.isValid()) {
        status = InterpreterStatus::ok();
        return false;
    }

    return true;
}