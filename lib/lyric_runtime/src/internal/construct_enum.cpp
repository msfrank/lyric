
#include <lyric_runtime/internal/construct_enum.h>
#include <tempo_utils/big_endian.h>

tempo_utils::Status
lyric_runtime::internal::construct_enum(
    tu_uint32 address,
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

    tempo_utils::Status status;
    auto existing = segmentManager->loadEnum(address, currentCoro, status);
    TU_RETURN_IF_NOT_OK (status);

    if (existing.type != DataCellType::INVALID) {
        TU_LOG_V << "loaded enum " << existing;
        return currentCoro->pushData(existing);
    }

    auto *sp = currentCoro->peekSP();
    TU_ASSERT (sp != nullptr);

    // determine the return IP and SP
    auto returnSegmentIndex = sp->getSegmentIndex();
    auto returnIP = currentCoro->peekIP();

    // resolve the enum descriptor
    auto receiver = segmentManager->resolveDescriptor(
        sp, lyric_object::LinkageSection::Enum, address, status);
    TU_RETURN_IF_NOT_OK (status);
    TU_ASSERT (receiver.isValid());

    auto *vtable = segmentManager->resolveEnumVirtualTable(receiver, status);
    TU_RETURN_IF_NOT_OK (status);
    TU_ASSERT (vtable != nullptr);

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

    auto *ctor = vtable->getCtor();
    auto ctorIndex = ctor->getCallIndex();
    auto *ctorSegment = ctor->getSegment();
    auto procOffset = ctor->getProcOffset();

    // verify the proc offset is within the segment bytecode
    auto *bytecodeData = ctorSegment->getBytecodeData();
    auto bytecodeSize = ctorSegment->getBytecodeSize();
    if (bytecodeSize <= procOffset)
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid proc offset");

    // calculate the stack guard
    auto stackGuard = currentCoro->dataStackSize();

    // read the call proc header
    const uint8_t *header = bytecodeData + procOffset;
    auto procSize = tempo_utils::read_u32_and_advance(header);
    auto numArguments = tempo_utils::read_u16_and_advance(header);
    auto numLocals = tempo_utils::read_u16_and_advance(header);
    auto numLexicals = tempo_utils::read_u16_and_advance(header);
    auto codeSize = procSize - 6;

    if (numArguments != 0)
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "unexpected arguments for enum ctor");
    if (numLexicals != 0)
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "unexpected lexicals for enum ctor");

    // construct the activation call frame
    CallCell frame(ctorIndex, ctorSegment->getSegmentIndex(),
        procOffset, returnSegmentIndex, returnIP, true,
        stackGuard, 0, 0, numLocals, 0,
        {}, ref);

    lyric_object::BytecodeIterator ip(header, codeSize);
    currentCoro->pushCall(frame, ip, ctorSegment);               // push the activation onto the call stack
    TU_LOG_V << "moved ip to " << ip;

    // reenter the interpreter to invoke the enum ctor
    TU_RETURN_IF_STATUS (interp->runSubinterpreter());

    // store the enum
    if (!segmentManager->storeEnum(address, ref, currentCoro, status))
        return status;

    TU_LOG_V << "constructed enum " << ref;
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(ref));

    return {};
}
