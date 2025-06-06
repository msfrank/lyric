
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

/**
 * Constructs a frame for the call specified by `callIndex` in the segment specified by `segment` and
 * pushes it onto the call stack. The `procOffset` is expected to be the index of the start of the call
 * proc in the bytecode of the segment.
 *
 * @param callIndex
 * @param segment
 * @param procOffset
 * @param args
 * @param currentCoro
 * @param status
 * @return
 */
bool
lyric_runtime::SubroutineManager::callProc(
    tu_uint32 callIndex,
    BytecodeSegment *segment,
    tu_uint32 procOffset,
    std::vector<DataCell> &args,
    bool returnsValue,
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
        returnIP, returnsValue, stackGuard, numArguments, numRest, numLocals, numLexicals, args);

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

static bool
call_static(
    lyric_runtime::StackfulCoroutine *currentCoro,
    lyric_runtime::BytecodeSegment *segment,
    tu_uint32 callIndex,
    lyric_object::CallWalker call,
    std::vector<lyric_runtime::DataCell> &args,
    tempo_utils::Status &status)
{
    // validate call descriptor
    if (call.getMode() == lyric_object::CallMode::Constructor) {
        status = lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid call flags");
        return false;
    }
    if (call.isBound()) {
        status = lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid call flags");
        return false;
    }

    tu_uint32 procOffset = call.getProcOffset();

    // proc offset must be within the segment bytecode
    auto *bytecodeData = segment->getBytecodeData();
    auto bytecodeSize = segment->getBytecodeSize();
    if (bytecodeSize <= procOffset) {
        status = lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid proc offset");
        return false;
    }

    const uint8_t *header = bytecodeData + procOffset;
    const lyric_runtime::BytecodeSegment *returnSP = currentCoro->peekSP();
    const lyric_object::BytecodeIterator returnIP = currentCoro->peekIP();
    bool returnsValue = !call.isNoReturn();
    auto stackGuard = currentCoro->dataStackSize();

    // read the call proc header
    auto procSize = tempo_utils::read_u32_and_advance(header);
    auto numArguments = tempo_utils::read_u16_and_advance(header);
    auto numLocals = tempo_utils::read_u16_and_advance(header);
    auto numLexicals = tempo_utils::read_u16_and_advance(header);
    auto codeSize = procSize - 6;

    // maximum number of args is 2^16
    if (std::numeric_limits<uint16_t>::max() <= args.size()) {
        status = lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "too many arguments");
        return false;
    }

    // all required args must be present
    if (args.size() < numArguments) {
        status = lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "not enough arguments");
        return false;
    }

    auto numRest = static_cast<uint16_t>(args.size()) - numArguments;

    // construct the activation call frame
    lyric_runtime::CallCell frame(callIndex, segment->getSegmentIndex(), procOffset, returnSP->getSegmentIndex(),
        returnIP, returnsValue, stackGuard, numArguments, numRest, numLocals, numLexicals, args);

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
                    status = lyric_runtime::InterpreterStatus::forCondition(
                        lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid lexical target");
                    return false;
            }
            found = true;                       // we found the lexical, break loop early
            break;
        }
        if (!found) {
            status = lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant, "missing lexical");
            return false;
        }
    }

    lyric_object::BytecodeIterator ip(header, codeSize);
    currentCoro->pushCall(frame, ip, segment);               // push the activation onto the call stack
    TU_LOG_V << "moved ip to " << ip;

    return true;
}

/**
 * Constructs a frame for the call specified by `address` and pushes it onto the call stack. The address
 * is resolved in relation to the SP of the `currentCoro`.
 *
 * @param address
 * @param args
 * @param currentCoro
 * @param status
 * @return
 */
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
    lyric_object::CallWalker call;

    auto *sp = currentCoro->peekSP();

    // resolve the segment and call proc
    if (lyric_object::IS_NEAR(address)) {
        segment = sp;
        auto object = segment->getObject().getObject();
        call = object.getCall(address);
        if (!call.isValid()) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "missing call");
            return false;
        }
        callIndex = address;
    } else {
        const auto *linkage = m_segmentManager->resolveLink(
            sp, lyric_object::GET_LINK_OFFSET(address), status);
        if (!linkage || linkage->linkage != lyric_object::LinkageSection::Call) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid call linkage");
            return false;
        }
        segment = m_segmentManager->getSegment(linkage->object);
        auto object = segment->getObject().getObject();
        call = object.getCall(linkage->value);
        if (!call.isValid()) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "missing call");
            return false;
        }
        callIndex = linkage->value;
    }

    return call_static(currentCoro, segment, callIndex, call, args, status);
}

bool
lyric_runtime::SubroutineManager::callStatic(
    const DataCell &descriptor,
    std::vector<DataCell> &args,
    StackfulCoroutine *currentCoro,
    tempo_utils::Status &status)
{
    TU_ASSERT (descriptor.type == DataCellType::CALL);
    TU_ASSERT (currentCoro != nullptr);

    auto *segment = descriptor.data.descriptor->getSegment();
    auto callIndex = descriptor.data.descriptor->getDescriptorIndex();
    auto object = segment->getObject().getObject();
    auto call = object.getCall(callIndex);
    if (!call.isValid()) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "missing call");
        return false;
    }

    return call_static(currentCoro, segment, callIndex, call, args, status);
}

/**
 * Constructs a frame for the call specified by `address` in the vtable of the specified `receiver` and
 * pushes it onto the call stack. The address is resolved in relation to the SP of the `currentCoro`.
 *
 * @param receiver
 * @param address
 * @param args
 * @param currentCoro
 * @param status
 * @return
 */
bool
lyric_runtime::SubroutineManager::callVirtual(
    const DataCell &receiver,
    tu_uint32 address,
    std::vector<DataCell> &args,
    StackfulCoroutine *currentCoro,
    tempo_utils::Status &status)
{
    TU_ASSERT (receiver.type == DataCellType::REF);
    TU_ASSERT (currentCoro != nullptr);

    auto *sp = currentCoro->peekSP();

    // resolve address to a descriptor
    auto descriptor = m_segmentManager->resolveDescriptor(
        sp, lyric_object::LinkageSection::Call, address, status);
    if (!descriptor.isValid())
        return false;

    // resolve the descriptor  to a vtable entry
    const auto *vtable = receiver.data.ref->getVirtualTable();
    TU_ASSERT (vtable != nullptr);
    const auto *method = vtable->getMethod(descriptor);
    if (method == nullptr) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "missing method");
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
        returnIP, method->returnsValue(), stackGuard, numArguments, numRest, numLocals, numLexicals, args, receiver);

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
    const DataCell &receiver,
    const DataCell &conceptDescriptor,
    tu_uint32 actionAddress,
    std::vector<DataCell> &args,
    StackfulCoroutine *currentCoro,
    tempo_utils::Status &status)
{
    TU_ASSERT (receiver.type == DataCellType::REF);
    TU_ASSERT (currentCoro != nullptr);

    auto *sp = currentCoro->peekSP();

    const auto *vtable = receiver.data.ref->getVirtualTable();
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
            InterpreterCondition::kRuntimeInvariant, "missing extension");
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
        returnIP, method->returnsValue(), stackGuard, numArguments, numRest, numLocals, numLexicals, args, receiver);

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
lyric_runtime::SubroutineManager::callExistential(
    const DataCell &receiver,
    const DataCell &existentialDescriptor,
    tu_uint32 methodAddress,
    std::vector<DataCell> &args,
    StackfulCoroutine *currentCoro,
    tempo_utils::Status &status)
{
    TU_ASSERT (receiver.isValid());
    TU_ASSERT (existentialDescriptor.isValid());
    TU_ASSERT (currentCoro != nullptr);

    auto *sp = currentCoro->peekSP();

    auto *etable = m_segmentManager->resolveExistentialTable(existentialDescriptor, status);
    if (etable == nullptr)
        return false;

    // resolve address to a descriptor
    auto callDescriptor = m_segmentManager->resolveDescriptor(
        sp, lyric_object::LinkageSection::Call, methodAddress, status);
    if (!callDescriptor.isValid())
        return false;

    // resolve the descriptor to an etable entry
    const auto *method = etable->getMethod(callDescriptor);
    if (method == nullptr) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "missing method");
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
        returnIP, method->returnsValue(), stackGuard, numArguments, numRest, numLocals, numLexicals, args, receiver);

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
        const auto *linkage = m_segmentManager->resolveLink(
            sp, lyric_object::GET_LINK_OFFSET(address), status);
        if (linkage == nullptr)
            return false;                   // failed to resolve dynamic link
        if (linkage->linkage != lyric_object::LinkageSection::Static) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid static linkage");
            return false;
        }
        segment = m_segmentManager->getSegment(linkage->object);
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
    auto returnsValue = !initializer.isNoReturn();

    std::vector<DataCell> args;
    return callProc(callIndex, segment, procOffset, args, returnsValue, currentCoro, status);
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

    // drop leftover temporaries from the stack
    if (currentCoro->dataStackSize() > frame.getStackGuard()) {
        // preserve the top most item as the return value if call returns a value
        if (frame.returnsValue()) {
            auto returnValue = currentCoro->popData();
            currentCoro->resizeDataStack(frame.getStackGuard());
            currentCoro->pushData(returnValue);
        } else {
            currentCoro->resizeDataStack(frame.getStackGuard());
        }
    }

    // return false if IP is not valid, which causes the program to halt
    if (!ip.isValid()) {
        status = InterpreterStatus::ok();
        return false;
    }

    return true;
}