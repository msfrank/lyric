
#include <lyric_runtime/internal/construct_new.h>
#include <tempo_utils/big_endian.h>

tempo_utils::Status
lyric_runtime::internal::construct_new(
    tu_uint32 address,
    tu_uint16 placement,
    tu_uint8 flags,
    StackfulCoroutine *currentCoro,
    SegmentManager *segmentManager,
    BytecodeInterpreter *interp,
    InterpreterState *state)
{
    TU_ASSERT (currentCoro != nullptr);
    TU_ASSERT (segmentManager != nullptr);
    TU_ASSERT (interp != nullptr);
    TU_ASSERT (state != nullptr);

    // pop constructor args off the stack
    std::vector<DataCell> args;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(placement, args));

    auto callFlags = lyric_object::GET_CALL_FLAGS(flags);

    // forward rest args from the enclosing activation if requested
    if (callFlags & lyric_object::CALL_FORWARD_REST) {
        CallCell *call;
        TU_RETURN_IF_NOT_OK (currentCoro->peekCall(&call));
        for (int i = 0; i < call->numRest(); i++) {
            args.push_back(call->getRest(i));
        }
    }

    auto *sp = currentCoro->peekSP();
    TU_ASSERT (sp != nullptr);

    // determine the return IP and SP so returnToCaller will restore correctly
    auto returnSegmentIndex = sp->getSegmentIndex();
    auto returnIP = currentCoro->peekIP();

    tempo_utils::Status status;

    // resolve the constructor call descriptor
    auto constructor = segmentManager->resolveDescriptor(
        sp, lyric_object::LinkageSection::Call, address, status);
    if (!constructor.isValid())
        return status;

    // determine the constructor receiver descriptor
    auto receiver = segmentManager->resolveReceiver(sp, address, status);
    if (!receiver.isValid())
        return status;

    // resolve the vtable for the receiver
    const VirtualTable *vtable;
    switch (receiver.type) {
        case DataCellType::CLASS: {
            vtable = segmentManager->resolveClassVirtualTable(receiver, status);
            if (vtable == nullptr)
                return status;
            break;
        }
        case DataCellType::ENUM: {
            vtable = segmentManager->resolveEnumVirtualTable(receiver, status);
            if (vtable == nullptr)
                return status;
            break;
        }
        case DataCellType::INSTANCE: {
            vtable = segmentManager->resolveInstanceVirtualTable(receiver, status);
            if (vtable == nullptr)
                return status;
            break;
        }
        case DataCellType::STRUCT: {
            vtable = segmentManager->resolveStructVirtualTable(receiver, status);
            if (vtable == nullptr)
                return status;
            break;
        }
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidReceiver,
                "invalid constructor receiver");
    }

    // find the allocator for the vtable
    NativeFunc allocator = nullptr;
    for (auto *curr = vtable; curr != nullptr; curr = curr->getParent()) {
        allocator = curr->getAllocator();
        if (allocator)
            break;
    }
    if (allocator == nullptr)
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "missing allocator");

    // invoke the allocator, placing the new instance on the top of the stack
    TU_RETURN_IF_NOT_OK (allocator(interp, state, vtable));

    // get the ref
    DataCell *top;
    TU_RETURN_IF_NOT_OK (currentCoro->peekData(&top));
    auto ref = *top;

    // load the required activation frame data
    auto *ctorSegment = constructor.data.descriptor->getSegment();
    auto ctorObject = ctorSegment->getObject();
    auto ctorIndex = constructor.data.descriptor->getDescriptorIndex();
    auto ctor = ctorObject.getCall(ctorIndex);
    const auto procOffset = ctor.getProcOffset();

    // parse the proc
    auto bytecode = ctorSegment->getBytecode();
    lyric_object::ProcInfo procInfo;
    TU_RETURN_IF_NOT_OK (lyric_object::parse_proc_info(bytecode, procOffset, procInfo));

    // calculate the stack guard
    auto stackGuard = currentCoro->dataStackSize();

    // process the arguments array
    tu_uint16 numRest;
    TU_RETURN_IF_NOT_OK (process_arguments(procInfo, args, numRest));

    // construct the activation call frame
    CallCell frame(ctorIndex, ctorSegment->getSegmentIndex(),
        procOffset, returnSegmentIndex, returnIP, true, stackGuard,
        procInfo.num_arguments, numRest, procInfo.num_locals, procInfo.num_lexicals, args, ref);

    // if any lexicals are present then import them
    if (procInfo.num_lexicals > 0) {
        TU_RETURN_IF_NOT_OK (import_lexicals_into_frame(procInfo, currentCoro, ctorSegment, frame));
    }

    lyric_object::BytecodeIterator ip(procInfo.code);
    currentCoro->pushCall(frame, ip, ctorSegment);               // push the activation onto the call stack
    TU_LOG_V << "moved ip to " << ip;

    return {};
}
