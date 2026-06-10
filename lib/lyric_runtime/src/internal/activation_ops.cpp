
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/internal/activation_ops.h>
#include <lyric_runtime/internal/construct_enum.h>
#include <lyric_runtime/internal/construct_instance.h>
#include <lyric_runtime/internal/construct_namespace.h>
#include <lyric_runtime/internal/construct_protocol.h>

tempo_utils::Status
lyric_runtime::internal::load(
    StackfulCoroutine *currentCoro,
    SegmentManager *segmentManager,
    SubroutineManager *subroutineManager,
    HeapManager *heapManager,
    InterpreterState *interpreterState,
    BytecodeInterpreter *interpreter,
    tu_uint32 address,
    tu_uint8 flags)
{
    const CallCell *activation;
    TU_RETURN_IF_NOT_OK (currentCoro->peekCall(&activation));

    switch (flags) {

        case lyric_object::LOAD_ARGUMENT: {
            auto argument = activation->getArgument(address);
            TU_LOG_V << "loaded argument " << argument;
            TU_RETURN_IF_NOT_OK (currentCoro->pushData(argument));
            return {};
        }

        case lyric_object::LOAD_LOCAL: {
            auto local = activation->getLocal(address);
            TU_LOG_V << "loaded local " << local;
            TU_RETURN_IF_NOT_OK (currentCoro->pushData(local));
            return {};

        }

        case lyric_object::LOAD_LEXICAL: {
            auto lexical = activation->getLexical(address);
            TU_LOG_V << "loaded lexical " << lexical;
            TU_RETURN_IF_NOT_OK (currentCoro->pushData(lexical));
            return {};
        }

        case lyric_object::LOAD_FIELD: {
            tempo_utils::Status status;
            auto field = segmentManager->resolveDescriptor(currentCoro->peekSP(),
                lyric_object::LinkageSection::Field, address, status);
            TU_RETURN_IF_NOT_OK (status);
            Operand receiver;
            TU_RETURN_IF_NOT_OK (currentCoro->popData(receiver));
            BaseRef *ref;
            if (!receiver.getRef(ref))
                return InterpreterStatus::forCondition(InterpreterCondition::kInvalidReceiver,
                    "invalid receiver for LOAD");
            Operand member;
            if (!ref->getField(field, member))
                return InterpreterStatus::forCondition(InterpreterCondition::kInvalidReceiver,
                    "field access failed");
            TU_LOG_V << "loaded value " << member << " in field " << field << " of receiver " << receiver;
            TU_RETURN_IF_NOT_OK (currentCoro->pushData(member));
            return {};
        }

        case lyric_object::LOAD_STATIC: {
            tempo_utils::Status status;
            auto value = segmentManager->loadStatic(address, currentCoro, status);
            TU_RETURN_IF_NOT_OK (status);
            if (!value.isValid()) {
                if (!subroutineManager->initStatic(address, currentCoro, status))
                    return status;
                // reenter the interpreter to invoke the init proc
                Operand init;
                TU_ASSIGN_OR_RETURN (init, interpreter->runSubinterpreter());
                // store the result of the init proc in the static cell
                if (!segmentManager->storeStatic(address, init, currentCoro, status))
                    return status;
                value = init;
            }
            TU_LOG_V << "loaded value " << value << " at static address " << address;
            TU_RETURN_IF_NOT_OK (currentCoro->pushData(value));
            return {};
        }

        case lyric_object::LOAD_INSTANCE: {
            TU_RETURN_IF_NOT_OK (internal::construct_instance(
                address, flags, currentCoro, segmentManager, interpreter, interpreterState));
            return {};
        }

        case lyric_object::LOAD_ENUM: {
            TU_RETURN_IF_NOT_OK (internal::construct_enum(
                address, flags, currentCoro, segmentManager, interpreter, interpreterState));
            return {};
        }

        case lyric_object::LOAD_PROTOCOL: {
            TU_RETURN_IF_NOT_OK (internal::construct_protocol(
                address, flags, currentCoro, segmentManager, heapManager, interpreterState));
            return {};
        }

        case lyric_object::LOAD_NAMESPACE: {
            TU_RETURN_IF_NOT_OK (internal::construct_namespace(
                address, flags, currentCoro, segmentManager, heapManager, interpreterState));
            return {};
        }
        default:
            return InterpreterStatus::forCondition(
                InterpreterCondition::kInvalidOperandFlagsAddressV1, "unknown LOAD flags");
    }
}

tempo_utils::Status
lyric_runtime::internal::store(
    StackfulCoroutine *currentCoro,
    SegmentManager *segmentManager,
    tu_uint32 address,
    tu_uint8 flags
    )
{
    CallCell *activation;
    TU_RETURN_IF_NOT_OK (currentCoro->peekCall(&activation));
    Operand value;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(value));

    switch (flags) {

        case lyric_object::STORE_ARGUMENT: {
            activation->setArgument(address, value);
            TU_LOG_V << "stored argument " << value;
            return {};
        }

        case lyric_object::STORE_LOCAL: {
            activation->setLocal(address, value);
            TU_LOG_V << "stored local " << value;
            return {};
        }

        case lyric_object::STORE_LEXICAL: {
            activation->setLexical(address, value);
            TU_LOG_V << "stored lexical " << value;
            return {};
        }

        case lyric_object::STORE_FIELD: {
            tempo_utils::Status status;
            auto field = segmentManager->resolveDescriptor(currentCoro->peekSP(),
                lyric_object::LinkageSection::Field, address, status);
            TU_RETURN_IF_NOT_OK (status);
            Operand receiver;
            TU_RETURN_IF_NOT_OK (currentCoro->popData(receiver));
            BaseRef *ref;
            if (!receiver.getRef(ref))
                return InterpreterStatus::forCondition(InterpreterCondition::kInvalidReceiver,
                    "invalid receiver for STORE");
            if (!ref->setField(field, value, nullptr))
                return InterpreterStatus::forCondition(InterpreterCondition::kInvalidReceiver,
                    "field update failed");
            TU_LOG_V << "stored value " << value << " in field " << field << " of receiver " << receiver;
            return {};
        }

        case lyric_object::STORE_STATIC: {
            tempo_utils::Status status;
            if (!segmentManager->storeStatic(address, value, currentCoro, status))
                return status;
            TU_LOG_V << "stored value " << value << " at static address " << address;
            return {};
        }

        default:
            return InterpreterStatus::forCondition(
                InterpreterCondition::kInvalidOperandFlagsAddressV1, "unknown STORE flags");
    }
}