
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/internal/activation_ops.h>
#include <lyric_runtime/internal/bitwise_ops.h>
#include <lyric_runtime/internal/call_ops.h>
#include <lyric_runtime/internal/compare_ops.h>
#include <lyric_runtime/internal/construct_enum.h>
#include <lyric_runtime/internal/construct_instance.h>
#include <lyric_runtime/internal/construct_namespace.h>
#include <lyric_runtime/internal/construct_new.h>
#include <lyric_runtime/internal/construct_protocol.h>
#include <lyric_runtime/internal/numeric_ops.h>
#include <lyric_runtime/internal/raise_exception.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/log_stream.h>

#define TIME_SLICE                      64
#define FAST_POLL_ITERATIONS            4
#define MAX_INTERPRETER_RECURSION       128

lyric_runtime::BytecodeInterpreter::BytecodeInterpreter(
    std::shared_ptr<InterpreterState> state,
    AbstractInspector *inspector)
    : m_state(state),
      m_inspector(inspector),
      m_sliceCounter(0),
      m_instructionCounter(0),
      m_recursionDepth(0)
{
    TU_ASSERT (m_state != nullptr);
}

tempo_utils::Result<lyric_runtime::InterpreterExit>
lyric_runtime::BytecodeInterpreter::run()
{
    InterpreterExit interpreterExit;

    TU_ASSERT (m_recursionDepth == 0);
    TU_ASSIGN_OR_RETURN (interpreterExit.mainReturn, runSubinterpreter());
    TU_ASSERT (m_recursionDepth == 0);

    interpreterExit.statusCode = m_state->getStatusCode();
    interpreterExit.interpreterStartEpochMillis = m_state->getLoadEpochMillis();
    interpreterExit.instructionCount = m_instructionCounter;

    return interpreterExit;
}

tempo_utils::Status
lyric_runtime::BytecodeInterpreter::interrupt()
{
    return InterpreterStatus::forCondition(
        InterpreterCondition::kRuntimeInvariant, "interrupt failed");
}

#define ON_ERROR_IF_NOT_OK(expr)                                        \
    do {                                                                \
        auto status__ = static_cast<tempo_utils::Status>(expr);         \
        if (status__.notOk())                                           \
            return onError(op, status__);                               \
    } while (0)


tempo_utils::Result<lyric_runtime::Operand>
lyric_runtime::BytecodeInterpreter::runSubinterpreter()
{
    RecursionLocker locker(this);
    if (locker.getRecursionDepth() > MAX_INTERPRETER_RECURSION)
        return InterpreterStatus::forCondition(InterpreterCondition::kExceededMaximumRecursion);

    auto *segmentManager = m_state->segmentManager();
    auto *heapManager = m_state->heapManager();
    auto *subroutineManager = m_state->subroutineManager();
    auto *systemScheduler = m_state->systemScheduler();
    auto *typeManager = m_state->typeManager();

    auto *currentCoro = m_state->currentCoro();
    TU_ASSERT (currentCoro != nullptr);
    currentCoro->pushGuard();

    for (;;) {

        // this will be set only if the current task has changed
        Task *nextReady = nullptr;

        m_instructionCounter++;
        m_sliceCounter++;

        // if time slice has been exceeded, then poll for events and schedule a new task
        if (m_sliceCounter > TIME_SLICE) {
            m_sliceCounter = 0;
            for (int i = 0; i < FAST_POLL_ITERATIONS; i++) {
                if (systemScheduler->poll())
                    break;
            }
            nextReady = systemScheduler->selectNextReady();
            currentCoro = m_state->currentCoro();
        }

        // block the interpreter polling for events until there is a ready task
        if (currentCoro == nullptr) {
            do {
                // perform blocking poll. this may not block if there is a ready task available.
                systemScheduler->blockingPoll();
                nextReady = systemScheduler->selectNextReady();
            } while (nextReady == nullptr);
            currentCoro = m_state->currentCoro();
            TU_ASSERT (currentCoro != nullptr);
        }

        // if we switched tasks then process all attached promises
        //if (nextReady) {
        //    nextReady->adaptPromises(this, m_state.get());
        //}

        // ensure the guard invariant for the current call stack is not violated
        if (!currentCoro->checkGuard())
            return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
                "stack invariant violation");

        // read the next bytecode op
        lyric_object::OpCell op;
        bool iteratorExhausted = !currentCoro->nextOp(op);
        if (iteratorExhausted)
            return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
                "no instruction available");

        // run inspector hook after processing op
        if (m_inspector) {
            auto status = m_inspector->beforeOp(op, this, m_state.get());
            if (!status.isOk())
                return status;
        }

        switch (op.opcode) {

            // no operation, continue to next instruction
            case lyric_object::Opcode::OP_NOOP:
                break;

            // push undef value onto the stack
            case lyric_object::Opcode::OP_UNDEF:
                currentCoro->pushData(Operand::undef());
                break;

            // push nil value onto the stack
            case lyric_object::Opcode::OP_NIL:
                currentCoro->pushData(Operand::nil());
                break;

            // push true value onto the stack
            case lyric_object::Opcode::OP_TRUE:
                currentCoro->pushData(Operand::fromBool(true));
                break;

            // push false value onto the stack
            case lyric_object::Opcode::OP_FALSE:
                currentCoro->pushData(Operand::fromBool(false));
                break;

            // push i64 value onto the stack
            case lyric_object::Opcode::OP_I64:
                currentCoro->pushData(Operand::fromI64(op.operands.immediate_i64.i64));
                break;

            // push dbl value onto the stack
            case lyric_object::Opcode::OP_DBL:
                currentCoro->pushData(Operand::fromF64(op.operands.immediate_dbl.dbl));
                break;

            // push chr value onto the stack
            case lyric_object::Opcode::OP_CHR:
                currentCoro->pushData(Operand::fromC32(op.operands.immediate_chr.chr));
                break;

            // push bytes ref onto the stack
            case lyric_object::Opcode::OP_BYTES: {
                auto status = heapManager->loadLiteralBytesOntoStack(op.operands.address_u32.address);
                if (status.notOk())
                    return onError(op, status);
                break;
            }

            // push string ref onto the stack
            case lyric_object::Opcode::OP_STRING: {
                auto status = heapManager->loadLiteralStringOntoStack(op.operands.address_u32.address);
                if (status.notOk())
                    return onError(op, status);
                break;
            }

            // push synthetic onto the stack
            case lyric_object::Opcode::OP_SYNTHETIC: {
                const CallCell *activation;
                ON_ERROR_IF_NOT_OK (currentCoro->peekCall(&activation));
                auto synthetic = op.operands.type_u8.type;
                switch (synthetic) {
                    case lyric_object::SYNTHETIC_THIS: {
                        auto receiver = activation->getReceiver();
                        TU_LOG_V << "loaded receiver " << receiver.toString();
                        ON_ERROR_IF_NOT_OK (currentCoro->pushData(receiver));
                        break;
                    }
                    case lyric_object::SYNTHETIC_REST: {
                        ON_ERROR_IF_NOT_OK (heapManager->loadRestOntoStack(*activation));
                        break;
                    }
                    default:
                        return onError(op, InterpreterStatus::forCondition(
                            InterpreterCondition::kInvalidOperandTypeV1, "unknown SYNTHETIC type"));
                }
                break;
            }

            // push descriptor value onto the stack
            case lyric_object::Opcode::OP_DESCRIPTOR: {
                auto section = op.operands.flags_u8_address_u32.flags;
                auto address = op.operands.flags_u8_address_u32.address;
                auto status = segmentManager->pushDescriptorOntoStack(
                    currentCoro->peekSP(), section, address, currentCoro);
                if (status.notOk())
                    return onError(op, status);
                break;
            }

            // load a value from the current activation frame and push it onto the stack
            case lyric_object::Opcode::OP_LOAD: {
                auto address = op.operands.flags_u8_address_u32.address;
                auto flags = op.operands.flags_u8_address_u32.flags;
                ON_ERROR_IF_NOT_OK (internal::load(
                    currentCoro, segmentManager, subroutineManager, heapManager, m_state.get(), this, address, flags));
                break;
            }

            // pop value from the stack and store it in the current activation frame
            case lyric_object::Opcode::OP_STORE: {
                auto address = op.operands.flags_u8_address_u32.address;
                auto flags = op.operands.flags_u8_address_u32.flags;
                ON_ERROR_IF_NOT_OK (internal::store(currentCoro, segmentManager, address, flags));
                break;
            }

            // pop index from the stack and push variadic argument in the current activation onto the top of the stack
            case lyric_object::Opcode::OP_VA_LOAD: {
                CallCell *activation;
                Operand load;
                tu_int64 offset;
                ON_ERROR_IF_NOT_OK (currentCoro->peekCall(&activation));
                ON_ERROR_IF_NOT_OK (currentCoro->popData(load));
                if (!load.getI64(offset))
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "invalid offset"));
                if (activation->numRest() <= offset)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "offset is out of range"));
                auto rest = activation->getRest(offset);
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(rest));
                break;
            }

            // push count of the variadic arguments in the current activation onto the top of the stack
            case lyric_object::Opcode::OP_VA_SIZE: {
                CallCell *activation;
                ON_ERROR_IF_NOT_OK (currentCoro->peekCall(&activation));
                auto size = Operand::fromI64(activation->numRest());
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(size));
                break;
            }

            // pop value from the stack and discard it
            case lyric_object::Opcode::OP_POP: {
                ON_ERROR_IF_NOT_OK (currentCoro->dropData());
                break;
            }

            // duplicate top value on the stack and push it onto the top of the stack
            case lyric_object::Opcode::OP_DUP: {
                Operand value;
                ON_ERROR_IF_NOT_OK (currentCoro->peekData(value));
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(value));
                break;
            }

            // duplicate the picked value on the stack and push onto the top of the stack
            case lyric_object::Opcode::OP_PICK: {
                auto offset = op.operands.offset_u16.offset;
                Operand value;
                ON_ERROR_IF_NOT_OK (currentCoro->peekData(value, offset));
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(value));
                break;
            }

            // remove the value at the specified offset from the stack
            case lyric_object::Opcode::OP_DROP: {
                auto offset = op.operands.offset_u16.offset;
                ON_ERROR_IF_NOT_OK (currentCoro->dropData(offset));
                break;
            }

            // pop 2 numeric values from the stack and add them, and push result onto the stack
            case lyric_object::Opcode::OP_ADD: {
                Operand lhs, rhs, result;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(rhs));
                ON_ERROR_IF_NOT_OK (currentCoro->popData(lhs));
                ON_ERROR_IF_NOT_OK (internal::add(lhs, rhs, result));
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(result));
                break;
            }

            // pop 2 numeric values from the stack and subtract them, and push result onto the stack
            case lyric_object::Opcode::OP_SUB: {
                Operand lhs, rhs, result;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(rhs));
                ON_ERROR_IF_NOT_OK (currentCoro->popData(lhs));
                ON_ERROR_IF_NOT_OK (internal::sub(lhs, rhs, result));
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(result));
                break;
            }

            // pop 2 numeric values from the stack and multiply them, and push result onto the stack
            case lyric_object::Opcode::OP_MUL: {
                Operand lhs, rhs, result;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(rhs));
                ON_ERROR_IF_NOT_OK (currentCoro->popData(lhs));
                ON_ERROR_IF_NOT_OK (internal::mul(lhs, rhs, result));
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(result));
                break;
            }

            // pop 2 numeric values from the stack and divide them, and push result onto the stack
            case lyric_object::Opcode::OP_DIV: {
                Operand lhs, rhs, result;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(rhs));
                ON_ERROR_IF_NOT_OK (currentCoro->popData(lhs));
                ON_ERROR_IF_NOT_OK (internal::div(lhs, rhs, result));
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(result));
                break;
            }

            // pop 1 numeric value from the stack and negate it, and push result onto the stack
            case lyric_object::Opcode::OP_NEG: {
                Operand element, result;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(element));
                ON_ERROR_IF_NOT_OK (internal::neg(element, result));
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(result));
                break;
            }

            // pop 2 bool values from the stack and compare them, and push result onto the stack
            case lyric_object::Opcode::OP_BOOL_CMP: {
                Operand lhs, rhs;
                bool l, r;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(rhs));
                if (!rhs.getBool(r))
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV2, "wrong type for rhs"));
                ON_ERROR_IF_NOT_OK (currentCoro->popData(lhs));
                if (!lhs.getBool(l))
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for lhs"));
                //result.type = OperandType::Int64;
                tu_int64 result;
                if (!l && r) {
                    result = -1;
                } else if (l && !r) {
                    result = 1;
                } else {
                    result = 0;
                }
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(Operand::fromI64(result)));
                break;
            }

            // pop 2 integer values from the stack and compare them, and push result onto the stack
            case lyric_object::Opcode::OP_I64_CMP: {
                Operand lhs, rhs, result;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(rhs));
                ON_ERROR_IF_NOT_OK (currentCoro->popData(lhs));
                ON_ERROR_IF_NOT_OK (internal::compare(lhs, rhs, result));
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(result));
                break;
            }

            // pop 2 float values from the stack and compare them, and push result onto the stack
            case lyric_object::Opcode::OP_DBL_CMP: {
                Operand lhs, rhs, result;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(rhs));
                ON_ERROR_IF_NOT_OK (currentCoro->popData(lhs));
                ON_ERROR_IF_NOT_OK (internal::compare(lhs, rhs, result));
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(result));
                break;
            }

            // pop 2 chr values from the stack and compare them, and push result onto the stack
            case lyric_object::Opcode::OP_CHR_CMP: {
                Operand lhs, rhs, result;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(rhs));
                ON_ERROR_IF_NOT_OK (currentCoro->popData(lhs));
                ON_ERROR_IF_NOT_OK (internal::compare(lhs, rhs, result));
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(result));
                break;
            }

            // pop two type descriptors from the stack and compare them, and push result onto the stack
            case lyric_object::Opcode::OP_TYPE_CMP: {
                Operand lhs, rhs;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(rhs));
                if (rhs.getType() != OperandType::Type)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV2, "wrong type for rhs"));
                ON_ERROR_IF_NOT_OK (currentCoro->popData(lhs));
                if (lhs.getType() != OperandType::Type)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for lhs"));
                auto compareTypesResult = typeManager->compareTypes(lhs, rhs);
                if (compareTypesResult.isStatus())
                    return onError(op, compareTypesResult.getStatus());
                tu_int64 result;
                switch (compareTypesResult.getResult()) {
                    case TypeComparison::EXTENDS:
                        result = -1;
                        break;
                    case TypeComparison::EQUAL:
                        result = 0;
                        break;
                    case TypeComparison::SUPER:
                    case TypeComparison::DISJOINT:
                        result = 1;
                        break;
                }
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(Operand::fromI64(result)));
                break;
            }

            // pop 2 bool values from the stack and perform logical AND, and push result onto the stack
            case lyric_object::Opcode::OP_LOGICAL_AND: {
                Operand lhs, rhs;
                bool l, r;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(rhs));
                if (!rhs.getBool(r))
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV2, "wrong type for rhs"));
                ON_ERROR_IF_NOT_OK (currentCoro->popData(lhs));
                if (!lhs.getBool(l))
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for lhs"));
                bool result = l && r;
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(Operand::fromBool(result)));
                break;
            }

            // pop 2 bool values from the stack and perform logical OR, and push result onto the stack
            case lyric_object::Opcode::OP_LOGICAL_OR: {
                Operand lhs, rhs;
                bool l, r;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(rhs));
                if (!rhs.getBool(r))
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV2, "wrong type for rhs"));
                ON_ERROR_IF_NOT_OK (currentCoro->popData(lhs));
                if (!lhs.getBool(l))
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for lhs"));
                bool result = l || r;
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(Operand::fromBool(result)));
                break;
            }

            // pop 1 bool value from the stack and perform logical NOT, and push result onto the stack
            case lyric_object::Opcode::OP_LOGICAL_NOT: {
                Operand element;
                bool e;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(element));
                if (!element.getBool(e))
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for value"));
                bool result = !e;
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(Operand::fromBool(result)));
                break;
            }

            case lyric_object::Opcode::OP_BITWISE_AND: {
                Operand lhs, rhs, result;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(rhs));
                ON_ERROR_IF_NOT_OK (currentCoro->popData(lhs));
                ON_ERROR_IF_NOT_OK (internal::bitwise_and(lhs, rhs, result));
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(result));
                break;
            }

            case lyric_object::Opcode::OP_BITWISE_OR: {
                Operand lhs, rhs, result;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(rhs));
                ON_ERROR_IF_NOT_OK (currentCoro->popData(lhs));
                ON_ERROR_IF_NOT_OK (internal::bitwise_or(lhs, rhs, result));
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(result));
                break;
            }

            case lyric_object::Opcode::OP_BITWISE_XOR: {
                Operand lhs, rhs, result;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(rhs));
                ON_ERROR_IF_NOT_OK (currentCoro->popData(lhs));
                ON_ERROR_IF_NOT_OK (internal::bitwise_xor(lhs, rhs, result));
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(result));
                break;
            }

            case lyric_object::Opcode::OP_BITWISE_RIGHT_SHIFT: {
                Operand element, count, result;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(count));
                ON_ERROR_IF_NOT_OK (currentCoro->popData(element));
                ON_ERROR_IF_NOT_OK (internal::bitwise_shr(element, count, result));
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(result));
                break;
            }

            case lyric_object::Opcode::OP_BITWISE_LEFT_SHIFT: {
                Operand element, count, result;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(count));
                ON_ERROR_IF_NOT_OK (currentCoro->popData(element));
                ON_ERROR_IF_NOT_OK (internal::bitwise_shl(element, count, result));
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(result));
                break;
            }

            // pop value from stack, and jump unconditionally
            case lyric_object::Opcode::OP_JUMP: {
                auto delta = op.operands.jump_i16.jump;
                if (!currentCoro->moveIP(delta))
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidOperandJumpV1, "invalid jump offset"));
                TU_LOG_V << "moved ip " << delta << " bytes to " << currentCoro->peekIP();
                break;
            }

            // pop value from stack, and jump to offset if value is Nil
            case lyric_object::Opcode::OP_IF_NIL: {
                Operand cmp;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(cmp));
                if (cmp.isNil()) {
                    auto delta = op.operands.jump_i16.jump;
                    if (!currentCoro->moveIP(delta))
                        return onError(op, InterpreterStatus::forCondition(
                            InterpreterCondition::kInvalidOperandJumpV1, "invalid jump offset"));
                    TU_LOG_V << "moved ip " << delta << " bytes to " << currentCoro->peekIP();
                }
                break;
            }

            // pop value from stack, and jump to offset if value is not Nil
            case lyric_object::Opcode::OP_IF_NOTNIL: {
                Operand cmp;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(cmp));
                if (!cmp.isNil()) {
                    auto delta = op.operands.jump_i16.jump;
                    if (!currentCoro->moveIP(delta))
                        return onError(op, InterpreterStatus::forCondition(
                            InterpreterCondition::kInvalidOperandJumpV1, "invalid jump offset"));
                    TU_LOG_V << "moved ip " << delta << " bytes to " << currentCoro->peekIP();
                }
                break;
            }

            // pop value from stack, and jump to offset if value is zero
            case lyric_object::Opcode::OP_IF_TRUE: {
                Operand cmp;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(cmp));
                bool b;
                if (!cmp.getBool(b))
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "value must be a boolean"));
                if (b) {
                    auto delta = op.operands.jump_i16.jump;
                    if (!currentCoro->moveIP(delta))
                        return onError(op, InterpreterStatus::forCondition(
                            InterpreterCondition::kInvalidOperandJumpV1, "invalid jump offset"));
                    TU_LOG_V << "moved ip " << delta << " bytes to " << currentCoro->peekIP();
                }
                break;
            }

            // pop value from stack, and jump to offset if value is not zero
            case lyric_object::Opcode::OP_IF_FALSE: {
                Operand cmp;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(cmp));
                bool b;
                if (!cmp.getBool(b))
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "value must be a boolean"));
                if (!b) {
                    auto delta = op.operands.jump_i16.jump;
                    if (!currentCoro->moveIP(delta))
                        return onError(op, InterpreterStatus::forCondition(
                            InterpreterCondition::kInvalidOperandJumpV1, "invalid jump offset"));
                    TU_LOG_V << "moved ip " << delta << " bytes to " << currentCoro->peekIP();
                }
                break;
            }

            // pop value from stack, and jump to offset if value is zero
            case lyric_object::Opcode::OP_IF_ZERO: {
                Operand cmp;
                bool result;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(cmp));
                ON_ERROR_IF_NOT_OK (internal::is_zero(cmp, result));
                if (result) {
                    auto delta = op.operands.jump_i16.jump;
                    if (!currentCoro->moveIP(delta))
                        return onError(op, InterpreterStatus::forCondition(
                            InterpreterCondition::kInvalidOperandJumpV1, "invalid jump offset"));
                    TU_LOG_V << "moved ip " << delta << " bytes to " << currentCoro->peekIP();
                }
                break;
            }

            // pop value from stack, and jump to offset if value is not zero
            case lyric_object::Opcode::OP_IF_NOTZERO: {
                Operand cmp;
                bool result;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(cmp));
                ON_ERROR_IF_NOT_OK (internal::is_not_zero(cmp, result));
                if (result) {
                    auto delta = op.operands.jump_i16.jump;
                    if (!currentCoro->moveIP(delta))
                        return onError(op, InterpreterStatus::forCondition(
                            InterpreterCondition::kInvalidOperandJumpV1, "invalid jump offset"));
                    TU_LOG_V << "moved ip " << delta << " bytes to " << currentCoro->peekIP();
                }
                break;
            }

            // pop value from stack, and jump to offset if value is less than zero
            case lyric_object::Opcode::OP_IF_LT: {
                Operand cmp;
                bool result;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(cmp));
                ON_ERROR_IF_NOT_OK (internal::is_less_than(cmp, result));
                if (result) {
                    auto delta = op.operands.jump_i16.jump;
                    if (!currentCoro->moveIP(delta))
                        return onError(op, InterpreterStatus::forCondition(
                            InterpreterCondition::kInvalidOperandJumpV1, "invalid jump offset"));
                    TU_LOG_V << "moved ip " << delta << " bytes to " << currentCoro->peekIP();
                }
                break;
            }

            // pop value from stack, and jump to offset if value is less than or equal to zero
            case lyric_object::Opcode::OP_IF_LE: {
                Operand cmp;
                bool result;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(cmp));
                ON_ERROR_IF_NOT_OK (internal::is_less_or_equal(cmp, result));
                if (result) {
                    auto delta = op.operands.jump_i16.jump;
                    if (!currentCoro->moveIP(delta))
                        return onError(op, InterpreterStatus::forCondition(
                            InterpreterCondition::kInvalidOperandJumpV1, "invalid jump offset"));
                    TU_LOG_V << "moved ip " << delta << " bytes to " << currentCoro->peekIP();
                }
                break;
            }

            // pop value from stack, and jump to offset if value is greater than zero
            case lyric_object::Opcode::OP_IF_GT: {
                Operand cmp;
                bool result;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(cmp));
                ON_ERROR_IF_NOT_OK (internal::is_greater_than(cmp, result));
                if (result) {
                    auto delta = op.operands.jump_i16.jump;
                    if (!currentCoro->moveIP(delta))
                        return onError(op, InterpreterStatus::forCondition(
                            InterpreterCondition::kInvalidOperandJumpV1, "invalid jump offset"));
                    TU_LOG_V << "moved ip " << delta << " bytes to " << currentCoro->peekIP();
                }
                break;
            }

            // pop value from stack, and jump to offset if value is greater than or equal o zero
            case lyric_object::Opcode::OP_IF_GE: {
                Operand cmp;
                bool result;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(cmp));
                ON_ERROR_IF_NOT_OK (internal::is_greater_or_equal(cmp, result));
                if (result) {
                    auto delta = op.operands.jump_i16.jump;
                    if (!currentCoro->moveIP(delta))
                        return onError(op, InterpreterStatus::forCondition(
                            InterpreterCondition::kInvalidOperandJumpV1, "invalid jump offset"));
                    TU_LOG_V << "moved ip " << delta << " bytes to " << currentCoro->peekIP();
                }
                break;
            }

            // load assembly specified by the literal address operand
            case lyric_object::Opcode::OP_IMPORT: {
                return onError(op, InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "OP_IMPORT unimplemented"));
            }

            // invoke the function specified by the static address operand
            case lyric_object::Opcode::OP_CALL_STATIC: {
                auto address = op.operands.flags_u8_address_u32_placement_u16.address;
                auto placementSize = op.operands.flags_u8_address_u32_placement_u16.placement;
                auto flags = op.operands.flags_u8_address_u32_placement_u16.flags;
                ON_ERROR_IF_NOT_OK (internal::call_static(
                    currentCoro, subroutineManager, address, placementSize, flags));
                break;
            }

            // execute the method specified by index into the vtable of the object on the top of the stack
            case lyric_object::Opcode::OP_CALL_VIRTUAL: {
                auto callAddress = op.operands.flags_u8_address_u32_placement_u16.address;
                auto placementSize = op.operands.flags_u8_address_u32_placement_u16.placement;
                auto flags = op.operands.flags_u8_address_u32_placement_u16.flags;
                ON_ERROR_IF_NOT_OK (internal::call_virtual(
                    currentCoro, subroutineManager, callAddress, placementSize, flags));
                break;
            }

            case lyric_object::Opcode::OP_CALL_CONCEPT: {
                auto actionAddress = op.operands.flags_u8_address_u32_placement_u16.address;
                auto placementSize = op.operands.flags_u8_address_u32_placement_u16.placement;
                auto flags = op.operands.flags_u8_address_u32_placement_u16.flags;
                ON_ERROR_IF_NOT_OK (internal::call_concept(
                    currentCoro, subroutineManager, actionAddress, placementSize, flags));
                break;
            }

            case lyric_object::Opcode::OP_CALL_STUB: {
                auto actionAddress = op.operands.flags_u8_address_u32_placement_u16.address;
                auto placementSize = op.operands.flags_u8_address_u32_placement_u16.placement;
                auto flags = op.operands.flags_u8_address_u32_placement_u16.flags;
                ON_ERROR_IF_NOT_OK (internal::call_stub(
                    currentCoro, subroutineManager, actionAddress, placementSize, flags));
                break;
            }

            case lyric_object::Opcode::OP_CALL_EXISTENTIAL: {
                auto callAddress = op.operands.flags_u8_address_u32_placement_u16.address;
                auto placementSize = op.operands.flags_u8_address_u32_placement_u16.placement;
                auto flags = op.operands.flags_u8_address_u32_placement_u16.flags;
                ON_ERROR_IF_NOT_OK (internal::call_existential(
                    currentCoro, subroutineManager, callAddress, placementSize, flags));
                break;
            }

            // return from the current activation
            case lyric_object::Opcode::OP_RETURN: {
                // if we reached the call stack guard then pop the guard
                bool reachedGuard = currentCoro->peekGuard() == currentCoro->callStackSize();
                if (reachedGuard)
                    currentCoro->popGuard();
                tempo_utils::Status status;
                // check if we reached the bottom of the call stack
                if (!subroutineManager->returnToCaller(currentCoro, status)) {
                    if (status.notOk())
                        return onError(op, status);
                    auto *currentTask = systemScheduler->currentTask();
                    // if we're executing the main task and we have no return address then halt
                    if (currentTask->isMainTask())
                        return onHalt(op);
                    // otherwise this is a worker task so terminate the task
                    systemScheduler->terminateTask(currentTask);
                    // clear the current coro so we select the next ready task
                    currentCoro = nullptr;
                    break;
                }
                // if the call stack is still valid and we reached a guard then return from the subinterpreter
                if (reachedGuard) {
                    Operand result;
                    if (currentCoro->dataStackSize() > 0) {
                        ON_ERROR_IF_NOT_OK (currentCoro->popData(result));
                    }
                    return result;
                }
                break;
            }
            case lyric_object::Opcode::OP_RAISE: {
                Operand exc;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(exc));
                if (exc.getType() != OperandType::Status)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "invalid exception"));
                ON_ERROR_IF_NOT_OK (internal::raise_exception(
                    op, exc, currentCoro, segmentManager, subroutineManager, typeManager));
                break;
            }

            // execute the specified trap, passing params from the stack, and push the result onto the stack.
            case lyric_object::Opcode::OP_TRAP: {
                auto flags = op.operands.flags_u8_address_u32.flags;
                auto address = op.operands.flags_u8_address_u32.address;
                if (flags & lyric_object::TRAP_INDEX_FOLLOWS) {
                    if (address != 0)
                        return onError(op, InterpreterStatus::forCondition(
                            InterpreterCondition::kInvalidOperandFlagsAddressV2, "invalid trap address operand"));
                    Operand trap;
                    ON_ERROR_IF_NOT_OK (currentCoro->popData(trap));
                    if (!trap.getU32(address))
                        return onError(op, InterpreterStatus::forCondition(
                            InterpreterCondition::kInvalidDataStackV1, "invalid trap index"));
                }
                TU_LOG_V << "trap index is " << address;
                auto *sp = currentCoro->peekSP();
                auto *trap = sp->getTrap(address);
                if (trap == nullptr)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "no trap found"));
                // invoke the trap
                auto status = trap->func(this, m_state.get(), nullptr);
                if (!status.isOk())
                    return onError(op, status);
                // ensure currentCoro is up to date
                currentCoro = m_state->currentCoro();
                break;
            }

            // invoke the constructor specified by the address operand
            case lyric_object::Opcode::OP_NEW: {
                auto address = op.operands.flags_u8_address_u32_placement_u16.address;
                auto placement = op.operands.flags_u8_address_u32_placement_u16.placement;
                auto flags = op.operands.flags_u8_address_u32_placement_u16.flags;

                auto status = internal::construct_new(
                    address, placement, flags, currentCoro, segmentManager, this, m_state.get());
                if (status.notOk())
                    return onError(op, status);
                break;
            }

            case lyric_object::Opcode::OP_TYPE_OF: {
                Operand value;
                ON_ERROR_IF_NOT_OK (currentCoro->popData(value));
                auto typeOfResult = typeManager->typeOf(value);
                if (typeOfResult.isStatus())
                    return onError(op, typeOfResult.getStatus());
                auto typeOf = typeOfResult.getResult();
                if (typeOf.getType() != OperandType::Type)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid type result"));
                ON_ERROR_IF_NOT_OK (currentCoro->pushData(typeOf));
                break;
            }

            // interrupt the interpreter.  if there is a value on the stack, return it, otherwise return nil.
            case lyric_object::Opcode::OP_INTERRUPT: {
                Operand result;
                if (!currentCoro->dataStackEmpty()) {
                    ON_ERROR_IF_NOT_OK (currentCoro->popData(result));
                }
                auto status = onInterrupt(result);
                // if status from interrupt handler is ok, then replace with generic status
                if (status.isOk())
                    return InterpreterStatus::forCondition(
                        InterpreterCondition::kInterrupted, "interrupted");
                return status;
            }

            // exit the interpreter.  if there is a value on the stack, return it, otherwise return nil.
            case lyric_object::Opcode::OP_HALT: {
                return onHalt(op);
            }

            // abort the interpreter.  if there is a value on the stack, return it, otherwise return nil.
            case lyric_object::Opcode::OP_ABORT: {
                return onError(op, InterpreterStatus::forCondition(InterpreterCondition::kAborted));
            }

            // unknown opcode
            default:
                return onError(op, InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "unknown instruction"));
        }

        // run inspector hook after processing op
        if (m_inspector) {
            auto status = m_inspector->afterOp(op, this, m_state.get());
            if (!status.isOk())
                return status;
        }
    }

    // if we are running reentrant
    if (currentCoro == nullptr)
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "no current coroutine");

    Operand result;
    auto status = currentCoro->popData(result);
    if (status.notOk())
        return status;
    return result;
}

lyric_runtime::InterpreterState *
lyric_runtime::BytecodeInterpreter::interpreterState() const
{
    return m_state.get();
}

lyric_runtime::AbstractInspector *
lyric_runtime::BytecodeInterpreter::interpreterInspector() const
{
    return m_inspector;
}

tu_uint16
lyric_runtime::BytecodeInterpreter::getSliceCounter() const
{
    return m_sliceCounter;
}

tu_uint64
lyric_runtime::BytecodeInterpreter::getInstructionCounter() const
{
    return m_instructionCounter;
}

int
lyric_runtime::BytecodeInterpreter::getRecursionDepth() const
{
    return m_recursionDepth;
}

void
lyric_runtime::BytecodeInterpreter::incrementRecursionDepth()
{
    m_recursionDepth++;
}

void
lyric_runtime::BytecodeInterpreter::decrementRecursionDepth()
{
    TU_ASSERT (m_recursionDepth > 0);
    m_recursionDepth--;
}

tempo_utils::Status
lyric_runtime::BytecodeInterpreter::onInterrupt(const Operand &cell)
{
    TU_LOG_VV << "interrupting interpreter";
    if (m_inspector)
        return m_inspector->onInterrupt(cell, this, m_state.get());
    return {};
}

tempo_utils::Result<lyric_runtime::Operand>
lyric_runtime::BytecodeInterpreter::onError(const lyric_object::OpCell &op, const tempo_utils::Status &status)
{
    TU_LOG_VV << status;
    if (m_inspector)
        return m_inspector->onError(op, status, this, m_state.get());
    return status;
}

tempo_utils::Result<lyric_runtime::Operand>
lyric_runtime::BytecodeInterpreter::onHalt(const lyric_object::OpCell &op)
{
    TU_LOG_VV << "halting interpreter";

    auto *currentCoro = m_state->currentCoro();
    Operand mainReturn;
    if (!currentCoro->dataStackEmpty()) {
        TU_RETURN_IF_NOT_OK (currentCoro->popData(mainReturn));
    }

    if (m_state->isActive()) {
        m_state->halt(tempo_utils::StatusCode::kOk);
    }

    if (m_inspector)
        return m_inspector->onHalt(op, mainReturn, this, m_state.get());
    return mainReturn;
}

lyric_runtime::RecursionLocker::RecursionLocker(lyric_runtime::BytecodeInterpreter *interp)
    : m_interp(interp)
{
    TU_ASSERT (m_interp != nullptr);
    m_interp->incrementRecursionDepth();
}

lyric_runtime::RecursionLocker::~RecursionLocker()
{
    m_interp->decrementRecursionDepth();
}

int
lyric_runtime::RecursionLocker::getRecursionDepth() const
{
    return m_interp->getRecursionDepth();
}
