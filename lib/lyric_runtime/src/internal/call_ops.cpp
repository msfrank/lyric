
#include <lyric_runtime/internal/call_ops.h>
#include <lyric_runtime/operand.h>

tempo_utils::Status
lyric_runtime::internal::call_static(
    StackfulCoroutine *currentCoro,
    SubroutineManager *subroutineManager,
    tu_uint32 address,
    tu_uint16 placement,
    tu_uint8 flags)
{
    std::vector<Operand> arguments;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(placement, arguments));

    // construct the activation call frame
    tempo_utils::Status status;
    if (!subroutineManager->callStatic(address, arguments, currentCoro, status))
        return status;

    return {};
}

tempo_utils::Status
lyric_runtime::internal::call_virtual(
    StackfulCoroutine *currentCoro,
    SubroutineManager *subroutineManager,
    tu_uint32 address,
    tu_uint16 placement,
    tu_uint8 flags)
{
    Operand receiver;
    std::vector<Operand> arguments;

    if (flags & lyric_object::CALL_RECEIVER_FOLLOWS) {
        // receiver comes after arguments so we pop receiver first
        TU_RETURN_IF_NOT_OK (currentCoro->popData(receiver));
        if (!receiver.isReference())
            return InterpreterStatus::forCondition(
                InterpreterCondition::kInvalidReceiver, "invalid receiver for virtual call");
        TU_RETURN_IF_NOT_OK (currentCoro->popData(placement, arguments));
    } else {
        // receiver comes before arguments so we pop placement first
        TU_RETURN_IF_NOT_OK (currentCoro->popData(placement, arguments));
        TU_RETURN_IF_NOT_OK (currentCoro->popData(receiver));
        if (!receiver.isReference())
            return InterpreterStatus::forCondition(
                InterpreterCondition::kInvalidReceiver, "invalid receiver for virtual call");
    }

    if (flags & lyric_object::CALL_FORWARD_REST) {
        CallCell *call;
        TU_RETURN_IF_NOT_OK (currentCoro->peekCall(&call));
        for (int i = 0; i < call->numRest(); i++) {
            arguments.push_back(call->getRest(i));
        }
    }

    // construct the activation call frame
    tempo_utils::Status status;
    if (!subroutineManager->callVirtual(receiver, address, arguments, currentCoro, status))
        return status;

    return {};
}

tempo_utils::Status
lyric_runtime::internal::call_stub(
    StackfulCoroutine *currentCoro,
    SubroutineManager *subroutineManager,
    tu_uint32 address,
    tu_uint16 placement,
    tu_uint8 flags)
{
    Operand receiver;
    std::vector<Operand> arguments;

    if (flags & lyric_object::CALL_RECEIVER_FOLLOWS) {
        // receiver comes after arguments so we pop receiver first
        TU_RETURN_IF_NOT_OK (currentCoro->popData(receiver));
        if (!receiver.isReference())
            return InterpreterStatus::forCondition(
                InterpreterCondition::kInvalidReceiver, "invalid receiver for stub call");
        TU_RETURN_IF_NOT_OK (currentCoro->popData(placement, arguments));
    } else {
        // receiver comes before arguments so we pop placement first
        TU_RETURN_IF_NOT_OK (currentCoro->popData(placement, arguments));
        TU_RETURN_IF_NOT_OK (currentCoro->popData(receiver));
        if (!receiver.isReference())
            return InterpreterStatus::forCondition(
                InterpreterCondition::kInvalidReceiver, "invalid receiver for stub call");
    }

    if (flags & lyric_object::CALL_FORWARD_REST) {
        CallCell *call;
        TU_RETURN_IF_NOT_OK (currentCoro->peekCall(&call));
        for (int i = 0; i < call->numRest(); i++) {
            arguments.push_back(call->getRest(i));
        }
    }

    // construct the activation call frame
    tempo_utils::Status status;
    if (!subroutineManager->callStub(receiver, address, arguments, currentCoro, status))
        return status;

    return {};
}

tempo_utils::Status
lyric_runtime::internal::call_concept(
    StackfulCoroutine *currentCoro,
    SubroutineManager *subroutineManager,
    tu_uint32 address,
    tu_uint16 placement,
    tu_uint8 flags)
{
    Operand receiver;
    std::vector<Operand> arguments;
    Operand descriptor;

    TU_RETURN_IF_NOT_OK (currentCoro->popData(descriptor));
    DescriptorEntry *conceptDescriptor;
    if (!descriptor.getDescriptor(conceptDescriptor))
        return InterpreterStatus::forCondition(
            InterpreterCondition::kInvalidDataStackV1, "invalid descriptor for concept call");
    if (conceptDescriptor->getLinkageSection() != lyric_object::LinkageSection::Concept)
        return InterpreterStatus::forCondition(
            InterpreterCondition::kInvalidDataStackV1, "invalid descriptor for concept call");

    if (flags & lyric_object::CALL_RECEIVER_FOLLOWS) {
        // receiver comes after arguments so we pop receiver first
        TU_RETURN_IF_NOT_OK (currentCoro->popData(receiver));
        if (!receiver.isReference())
            return InterpreterStatus::forCondition(
                InterpreterCondition::kInvalidReceiver, "invalid receiver for concept call");
        TU_RETURN_IF_NOT_OK (currentCoro->popData(placement, arguments));
    } else {
        // receiver comes before arguments so we pop placement first
        TU_RETURN_IF_NOT_OK (currentCoro->popData(placement, arguments));
        TU_RETURN_IF_NOT_OK (currentCoro->popData(receiver));
        if (!receiver.isReference())
            return InterpreterStatus::forCondition(
                InterpreterCondition::kInvalidReceiver, "invalid receiver for concept call");
    }

    // construct the activation call frame
    tempo_utils::Status status;
    if (!subroutineManager->callConcept(receiver, descriptor, address, arguments, currentCoro, status))
        return status;

    return {};
}

tempo_utils::Status
lyric_runtime::internal::call_existential(
    StackfulCoroutine *currentCoro,
    SubroutineManager *subroutineManager,
    tu_uint32 address,
    tu_uint16 placement,
    tu_uint8 flags)
{
    Operand receiver;
    std::vector<Operand> arguments;
    Operand descriptor;

    TU_RETURN_IF_NOT_OK (currentCoro->popData(descriptor));
    DescriptorEntry *existentialDescriptor;
    if (!descriptor.getDescriptor(existentialDescriptor))
        return InterpreterStatus::forCondition(
            InterpreterCondition::kInvalidDataStackV1, "invalid descriptor for existential call");
    if (existentialDescriptor->getLinkageSection() != lyric_object::LinkageSection::Existential)
        return InterpreterStatus::forCondition(
            InterpreterCondition::kInvalidDataStackV1, "invalid descriptor for existential call");

    TU_RETURN_IF_NOT_OK (currentCoro->popData(placement, arguments));
    TU_RETURN_IF_NOT_OK (currentCoro->popData(receiver));

    // construct the activation call frame
    tempo_utils::Status status;
    if (!subroutineManager->callExistential(
        receiver, descriptor, address, arguments, currentCoro, status))
        return status;

    return {};
}