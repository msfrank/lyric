
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

    // maximum number of args is 2^16
    if (std::numeric_limits<uint16_t>::max() <= args.size())
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "too many arguments");

    // all required args must be present
    if (args.size() < numArguments)
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "not enough arguments");

    auto numRest = static_cast<uint16_t>(args.size()) - numArguments;

    // construct the activation call frame
    CallCell frame(ctorIndex, ctorSegment->getSegmentIndex(),
        procOffset, returnSegmentIndex, returnIP, true,
        stackGuard, numArguments, numRest, numLocals, numLexicals,
        args, ref);

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

            if (ancestor.getCallSegment() != ctorSegment->getSegmentIndex())    // lexical must exist in callee segment
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
                    return InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid lexical target");
            }
            found = true;                       // we found the lexical, break loop early
            break;
        }
        if (!found)
            return InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "missing lexical");
    }

    lyric_object::BytecodeIterator ip(header, codeSize);
    currentCoro->pushCall(frame, ip, ctorSegment);               // push the activation onto the call stack
    TU_LOG_V << "moved ip to " << ip;

    return {};
}
