
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/call_cell.h>
#include <lyric_runtime/stackful_coroutine.h>
#include <lyric_runtime/subroutine_manager.h>
#include <tempo_utils/big_endian.h>

#include <lyric_runtime/bytes_ref.h>
#include <lyric_runtime/rest_ref.h>
#include <lyric_runtime/status_ref.h>
#include <lyric_runtime/string_ref.h>
#include <lyric_runtime/url_ref.h>

lyric_runtime::SubroutineManager::SubroutineManager(SegmentManager *segmentManager)
    : m_segmentManager(segmentManager)
{
    TU_ASSERT (m_segmentManager != nullptr);
}

/**
 * validate the argument count and determine the number of rest arguments.
 */
tempo_utils::Status
lyric_runtime::process_arguments(
    const lyric_object::ProcInfo &procInfo,
    const std::vector<DataCell> &args,
    tu_uint16 &numRest)
{

    // maximum number of args is 2^16
    if (std::numeric_limits<tu_uint16>::max() <= args.size())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "too many arguments");

    // all required args must be present
    if (args.size() < procInfo.num_arguments)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "not enough arguments");

    // any additional arguments are considered rest args
    auto remaining = args.size() - procInfo.num_arguments;
    numRest = static_cast<tu_uint16>(remaining);

    return {};
}

/**
 * import each lexical from the latest activation and insert the lexical into the frame.
 */
tempo_utils::Status
lyric_runtime::import_lexicals_into_frame(
    const lyric_object::ProcInfo &procInfo,
    const StackfulCoroutine *currentCoro,
    const BytecodeSegment *segment,
    CallCell &frame)
{
    std::vector<lyric_object::ProcLexical> lexicals;
    TU_RETURN_IF_NOT_OK (lyric_object::parse_lexicals_table(procInfo, lexicals));

    for (int i = 0; i < lexicals.size(); i++) {
        const auto &lexical = lexicals.at(i);

        bool found = false;
        for (auto iterator = currentCoro->callsBegin(); iterator != currentCoro->callsEnd(); iterator++) {
            const auto &ancestor = *iterator;

            if (ancestor.getCallSegment() != segment->getSegmentIndex())    // lexical must exist in callee segment
                continue;
            if (ancestor.getCallIndex() != lexical.activation_call)         // frame does not match activation call
                continue;
            switch (lexical.lexical_target) {
                case lyric_object::LEXICAL_ARGUMENT:
                    frame.setLexical(i, ancestor.getArgument(lexical.target_offset));
                    break;
                case lyric_object::LEXICAL_LOCAL:
                    frame.setLexical(i, ancestor.getLocal(lexical.target_offset));
                    break;
                default:
                    return lyric_runtime::InterpreterStatus::forCondition(
                        lyric_runtime::InterpreterCondition::kRuntimeInvariant,
                        "invalid lexical target at offset {}", i);
            }
            found = true;                       // we found the lexical, break loop early
            break;
        }
        if (!found)
            return lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant,
                "missing lexical");
    }

    return {};
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

    auto bytecode = segment->getBytecode();
    lyric_object::ProcInfo procInfo;
    status = lyric_object::parse_proc_info(bytecode, procOffset, procInfo);
    if (status.notOk())
        return false;

    const BytecodeSegment *returnSP = currentCoro->peekSP();
    const lyric_object::BytecodeIterator returnIP = currentCoro->peekIP();
    auto stackGuard = currentCoro->dataStackSize();

    // process the arguments array
    tu_uint16 numRest;
    status = process_arguments(procInfo, args, numRest);
    if (status.notOk())
        return false;

    // construct the activation call frame
    CallCell frame(callIndex, segment->getSegmentIndex(), procOffset,
        returnSP->getSegmentIndex(), returnIP, returnsValue, stackGuard,
        procInfo.num_arguments, numRest, procInfo.num_locals, procInfo.num_lexicals, args);

    // if any lexicals are present then import them
    if (procInfo.num_lexicals > 0) {
        status = import_lexicals_into_frame(procInfo, currentCoro, segment, frame);
        if (status.notOk())
            return false;
    }

    lyric_object::BytecodeIterator ip(procInfo.code);
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

    auto bytecode = segment->getBytecode();
    tu_uint32 procOffset = call.getProcOffset();
    lyric_object::ProcInfo procInfo;
    status = lyric_object::parse_proc_info(bytecode, procOffset, procInfo);
    if (status.notOk())
        return false;

    const lyric_runtime::BytecodeSegment *returnSP = currentCoro->peekSP();
    const lyric_object::BytecodeIterator returnIP = currentCoro->peekIP();
    bool returnsValue = !call.isNoReturn();
    auto stackGuard = currentCoro->dataStackSize();

    // process the arguments array
    tu_uint16 numRest;
    status = lyric_runtime::process_arguments(procInfo, args, numRest);
    if (status.notOk())
        return false;

    // construct the activation call frame
    lyric_runtime::CallCell frame(callIndex, segment->getSegmentIndex(), procOffset,
        returnSP->getSegmentIndex(), returnIP, returnsValue, stackGuard,
        procInfo.num_arguments, numRest, procInfo.num_locals, procInfo.num_lexicals, args);

    // if any lexicals are present then import them
    if (procInfo.num_lexicals > 0) {
        status = lyric_runtime::import_lexicals_into_frame(procInfo, currentCoro, segment, frame);
        if (status.notOk())
            return false;
    }

    lyric_object::BytecodeIterator ip(procInfo.code);
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
        auto object = segment->getObject();
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
        auto object = segment->getObject();
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
    auto object = segment->getObject();
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
    TU_ASSERT (currentCoro != nullptr);

    auto *sp = currentCoro->peekSP();

    // get method resolver for receiver
    const AbstractMethodResolver *resolver;
    switch (receiver.type) {
        case DataCellType::BYTES:
            resolver = receiver.data.bytes->getMethodResolver();
            break;
        case DataCellType::REF:
            resolver = receiver.data.ref->getMethodResolver();
            break;
        case DataCellType::REST:
            resolver = receiver.data.rest->getMethodResolver();
            break;
        case DataCellType::STATUS:
            resolver = receiver.data.status->getMethodResolver();
            break;
        case DataCellType::STRING:
            resolver = receiver.data.str->getMethodResolver();
            break;
        case DataCellType::URL:
            resolver = receiver.data.url->getMethodResolver();
            break;
        default:
            resolver = nullptr;
            break;
    }
    if (resolver == nullptr) {
        status = InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "cannot resolve method; invalid receiver {}", receiver.toString());
        return false;
    }

    // resolve address to a call descriptor
    auto descriptor = m_segmentManager->resolveDescriptor(
        sp, lyric_object::LinkageSection::Call, address, status);
    if (!descriptor.isValid())
        return false;

    // resolve the descriptor to a method
    const auto *method = resolver->getMethod(descriptor);
    if (method == nullptr) {
        status = InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "failed to call method; missing method");
        return false;
    }

    auto *segment = method->getSegment();
    const auto callIndex = method->getCallIndex();
    tu_uint32 procOffset = method->getProcOffset();
    bool returnsValue = method->returnsValue();

    auto bytecode = segment->getBytecode();
    lyric_object::ProcInfo procInfo;
    status = lyric_object::parse_proc_info(bytecode, procOffset, procInfo);
    if (status.notOk())
        return false;

    const BytecodeSegment *returnSP = sp;
    const lyric_object::BytecodeIterator returnIP = currentCoro->peekIP();
    auto stackGuard = currentCoro->dataStackSize();

    // process the arguments array
    tu_uint16 numRest;
    status = process_arguments(procInfo, args, numRest);
    if (status.notOk())
        return false;

    // construct the activation call frame
    CallCell frame(callIndex, segment->getSegmentIndex(), procOffset,
        returnSP->getSegmentIndex(), returnIP, returnsValue, stackGuard,
        procInfo.num_arguments, numRest, procInfo.num_locals, procInfo.num_lexicals, args, receiver);

    // if any lexicals are present then import them
    if (procInfo.num_lexicals > 0) {
        status = import_lexicals_into_frame(procInfo, currentCoro, segment, frame);
        if (status.notOk())
            return false;
    }

    lyric_object::BytecodeIterator ip(procInfo.code);
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
    tempo_utils::Status &status) {
    TU_ASSERT (currentCoro != nullptr);

    auto *sp = currentCoro->peekSP();

    // get extension resolver for receiver
    const AbstractExtensionResolver *resolver;
    switch (receiver.type) {
        case DataCellType::BYTES:
            resolver = receiver.data.bytes->getExtensionResolver();
            break;
        case DataCellType::REF:
            resolver = receiver.data.ref->getExtensionResolver();
            break;
        case DataCellType::REST:
            resolver = receiver.data.rest->getExtensionResolver();
            break;
        case DataCellType::STRING:
            resolver = receiver.data.str->getExtensionResolver();
            break;
        case DataCellType::URL:
            resolver = receiver.data.url->getExtensionResolver();
            break;
        default:
            resolver = nullptr;
            break;
    }
    if (resolver == nullptr) {
        status = InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "cannot resolve extension; invalid receiver {}", receiver.toString());
        return false;
    }

    // resolve address to an action descriptor
    auto actionDescriptor = m_segmentManager->resolveDescriptor(
        sp, lyric_object::LinkageSection::Action, actionAddress, status);
    if (!actionDescriptor.isValid())
        return false;

    // resolve the descriptor to a method
    const auto *method = resolver->getExtension(conceptDescriptor, actionDescriptor);
    if (method == nullptr) {
        status = InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "failed to call extension; missing extension");
        return false;
    }

    auto *segment = method->getSegment();
    const auto callIndex = method->getCallIndex();
    const auto procOffset = method->getProcOffset();
    bool returnsValue = method->returnsValue();

    auto bytecode = segment->getBytecode();
    lyric_object::ProcInfo procInfo;
    status = lyric_object::parse_proc_info(bytecode, procOffset, procInfo);
    if (status.notOk())
        return false;

    const BytecodeSegment *returnSP = sp;
    const lyric_object::BytecodeIterator returnIP = currentCoro->peekIP();
    auto stackGuard = currentCoro->dataStackSize();

    // process the arguments array
    tu_uint16 numRest;
    status = process_arguments(procInfo, args, numRest);
    if (status.notOk())
        return false;

    // construct the activation call frame
    CallCell frame(callIndex, segment->getSegmentIndex(), procOffset,
        returnSP->getSegmentIndex(), returnIP, returnsValue, stackGuard,
        procInfo.num_arguments, numRest, procInfo.num_locals, procInfo.num_lexicals, args, receiver);

    // if any lexicals are present then import them
    if (procInfo.num_lexicals > 0) {
        status = import_lexicals_into_frame(procInfo, currentCoro, segment, frame);
        if (status.notOk())
            return false;
    }

    lyric_object::BytecodeIterator ip(procInfo.code);
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
    bool returnsValue = method->returnsValue();

    auto bytecode = segment->getBytecode();
    lyric_object::ProcInfo procInfo;
    status = lyric_object::parse_proc_info(bytecode, procOffset, procInfo);
    if (status.notOk())
        return false;

    const BytecodeSegment *returnSP = sp;
    const lyric_object::BytecodeIterator returnIP = currentCoro->peekIP();
    auto stackGuard = currentCoro->dataStackSize();

    // process the arguments array
    tu_uint16 numRest;
    status = process_arguments(procInfo, args, numRest);
    if (status.notOk())
        return false;

    // construct the activation call frame
    CallCell frame(callIndex, segment->getSegmentIndex(), procOffset,
        returnSP->getSegmentIndex(), returnIP, returnsValue, stackGuard,
        procInfo.num_arguments, numRest, procInfo.num_locals, procInfo.num_lexicals, args, receiver);

    // if any lexicals are present then import them
    if (procInfo.num_lexicals > 0) {
        status = import_lexicals_into_frame(procInfo, currentCoro, segment, frame);
        if (status.notOk())
            return false;
    }

    lyric_object::BytecodeIterator ip(procInfo.code);
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
    lyric_object::LyricObject object;
    lyric_object::StaticWalker static_;

    auto *sp = currentCoro->peekSP();

    if (lyric_object::IS_NEAR(address)) {
        segment = sp;
        object = sp->getObject();
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
        object = segment->getObject();
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
    CallCell frame;
    status = currentCoro->popCall(frame);
    if (status.notOk())
        return false;

    auto ip = frame.getReturnIP();
    auto sp = m_segmentManager->getSegment(frame.getReturnSegment());
    currentCoro->transferControl(ip, sp);

    // drop leftover temporaries from the stack
    if (currentCoro->dataStackSize() > frame.getStackGuard()) {
        // preserve the top most item as the return value if call returns a value
        if (frame.returnsValue()) {
            DataCell returnValue;
            status = currentCoro->popData(returnValue);
            if (status.notOk())
                return false;
            status = currentCoro->resizeDataStack(frame.getStackGuard());
            if (status.notOk())
                return false;
            status = currentCoro->pushData(returnValue);
            if (status.notOk())
                return false;
        } else {
            status = currentCoro->resizeDataStack(frame.getStackGuard());
            if (status.notOk())
                return false;
        }
    }

    // return false if IP is not valid, which causes the program to halt
    if (!ip.isValid()) {
        status = {};
        return false;
    }

    return true;
}