
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/data_cell.h>
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
    interpreterExit.interpreterStartEpochMillis = m_state->getReloadEpochMillis();
    interpreterExit.instructionCount = m_instructionCounter;

    return interpreterExit;
}

tempo_utils::Status
lyric_runtime::BytecodeInterpreter::interrupt()
{
    return InterpreterStatus::forCondition(
        InterpreterCondition::kRuntimeInvariant, "interrupt failed");
}

tempo_utils::Result<lyric_runtime::DataCell>
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
        if (nextReady) {
            nextReady->adaptPromises(this, m_state.get());
        }

        // ensure the guard invariant for the current call stack is not violated
        if (!currentCoro->checkGuard())
            return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
                "stack invariant violation");

        // read the next bytecode op
        lyric_object::OpCell op;
        if (!currentCoro->nextOp(op)) {

            // if there is no next op but there is a guard then we have reached the end of the current
            // call but possibly not the end of the current task, so try returning to the caller
            if (currentCoro->guardStackSize() > 0) {
                InterpreterStatus status;

                // if we reached the call stack guard then pop the guard
                bool reachedGuard = currentCoro->peekGuard() == currentCoro->callStackSize();
                if (reachedGuard)
                    currentCoro->popGuard();

                // the end of the iterator means we return to the caller
                if (subroutineManager->returnToCaller(currentCoro, status)) {
                    // if we did not reach a guard then jump back to the top of the interpreter loop
                    if (!reachedGuard)
                        continue;
                    // otherwise fall through
                }

                // if result is false and status is not ok, then return error
                if (status.notOk())
                    return status;
            }

            // at this point we know the current task is finished. we behave differently depending
            // on whether the current task is the main task or a worker task.
            Task *currentTask = systemScheduler->currentTask();

            // if current task is the main task then halt
            if (currentTask->getTaskType() == TaskType::Main)
                return onHalt(op);

            // otherwise this is a worker task, get the result and terminate the task
            systemScheduler->terminateTask(currentTask);

            // clear the current coro and jump back to the top of the loop
            currentCoro = nullptr;
            continue;
        }

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
                currentCoro->pushData(DataCell::undef());
                break;

            // push nil value onto the stack
            case lyric_object::Opcode::OP_NIL:
                currentCoro->pushData(DataCell::nil());
                break;

            // push true value onto the stack
            case lyric_object::Opcode::OP_TRUE:
                currentCoro->pushData(DataCell(true));
                break;

            // push false value onto the stack
            case lyric_object::Opcode::OP_FALSE:
                currentCoro->pushData(DataCell(false));
                break;

            // push i64 value onto the stack
            case lyric_object::Opcode::OP_I64:
                currentCoro->pushData(DataCell(op.operands.immediate_i64.i64));
                break;

            // push dbl value onto the stack
            case lyric_object::Opcode::OP_DBL:
                currentCoro->pushData(DataCell(op.operands.immediate_dbl.dbl));
                break;

            // push chr value onto the stack
            case lyric_object::Opcode::OP_CHR:
                currentCoro->pushData(DataCell(op.operands.immediate_chr.chr));
                break;

            // push literal onto the stack
            case lyric_object::Opcode::OP_LITERAL: {
                auto status = segmentManager->pushLiteralOntoStack(
                    currentCoro->peekSP(), op.operands.address_u32.address, currentCoro);
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

            // push url ref onto the stack
            case lyric_object::Opcode::OP_URL: {
                auto status = heapManager->loadLiteralUrlOntoStack(op.operands.address_u32.address);
                if (status.notOk())
                    return onError(op, status);
                break;
            }

            // push synthetic onto the stack
            case lyric_object::Opcode::OP_SYNTHETIC: {
                const auto &activation = currentCoro->peekCall();
                auto synthetic = op.operands.type_u8.type;
                switch (synthetic) {
                    case lyric_object::SYNTHETIC_THIS: {
                        auto receiver = activation.getReceiver();
                        TU_LOG_V << "loaded receiver " << receiver.toString();
                        currentCoro->pushData(receiver);
                        break;
                    }
                    case lyric_object::SYNTHETIC_REST: {
                        auto status = heapManager->loadRestOntoStack(activation);
                        if (status.notOk())
                            return onError(op, status);
                        break;
                    }
                    default:
                        return onError(op, InterpreterStatus::forCondition(InterpreterCondition::kInvalidOperandTypeV1,
                            "unknown SYNTHETIC type"));
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
                auto flags = op.operands.flags_u8_address_u32.flags;
                auto index = op.operands.flags_u8_address_u32.address;
                const auto &activation = currentCoro->peekCall();
                switch (flags) {
                    case lyric_object::LOAD_ARGUMENT: {
                        auto argument = activation.getArgument(index);
                        TU_LOG_V << "loaded argument " << argument;
                        currentCoro->pushData(argument);
                        break;
                    }
                    case lyric_object::LOAD_LOCAL: {
                        auto local = activation.getLocal(index);
                        TU_LOG_V << "loaded local " << local;
                        currentCoro->pushData(local);
                        break;
                    }
                    case lyric_object::LOAD_LEXICAL: {
                        auto lexical = activation.getLexical(index);
                        TU_LOG_V << "loaded lexical " << lexical;
                        currentCoro->pushData(lexical);
                        break;
                    }
                    case lyric_object::LOAD_FIELD: {
                        InterpreterStatus status;
                        auto field = segmentManager->resolveDescriptor(currentCoro->peekSP(),
                            lyric_object::LinkageSection::Field, index, status);
                        if (!field.isValid())
                            return onError(op, status);
                        auto receiver = currentCoro->popData();
                        if (receiver.type != DataCellType::REF)
                            return onError(op, InterpreterStatus::forCondition(InterpreterCondition::kInvalidReceiver,
                                "invalid receiver for LOAD"));
                        auto *instance = receiver.data.ref;
                        auto member = instance->getField(field);
                        TU_LOG_V << "loaded value " << member << " in field " << field << " of receiver " << receiver;
                        currentCoro->pushData(member);
                        break;
                    }
                    case lyric_object::LOAD_STATIC: {
                        InterpreterStatus status;
                        auto value = segmentManager->loadStatic(index, currentCoro, status);
                        if (value.type == DataCellType::INVALID) {
                            if (status.notOk())
                                return onError(op, status);
                            if (!subroutineManager->initStatic(index, currentCoro, status))
                                return onError(op, status);
                            // reenter the interpreter to invoke the init proc
                            auto initProcResult = runSubinterpreter();
                            if (initProcResult.isStatus())
                                return initProcResult;
                            // store the result of the init proc in the static cell
                            value = initProcResult.getResult();
                            if (!segmentManager->storeStatic(index, value, currentCoro, status))
                                return onError(op, status);
                        }
                        TU_LOG_V << "loaded value " << value << " at static address " << index;
                        currentCoro->pushData(value);
                        break;
                    }
                    case lyric_object::LOAD_INSTANCE: {
                        tempo_utils::Status status;
                        auto value = segmentManager->loadInstance(index, currentCoro, status);
                        if (value.type == DataCellType::INVALID) {
                            if (status.notOk())
                                return onError(op, status);
                            // invoke the allocator
                            auto *allocator = heapManager->prepareNew(
                                lyric_object::NEW_INSTANCE, index, status);
                            if (allocator == nullptr)
                                return onError(op, status);
                            status = allocator(this, m_state.get());
                            if (status.notOk())
                                return onError(op, status);
                            if (!subroutineManager->returnToCaller(currentCoro, status))
                                return onError(op, status);
                            // capture the uninitialized instance
                            value = currentCoro->peekData();
                            // push the ctor frame onto the call stack
                            std::vector<DataCell> noArguments;
                            if (!heapManager->constructNew(noArguments, status))
                                return onError(op, status);
                            // reenter the interpreter to invoke the instance ctor
                            auto initInstanceResult = runSubinterpreter();
                            if (initInstanceResult.isStatus())
                                return initInstanceResult;
                            // store the instance
                            if (!segmentManager->storeInstance(index, value, currentCoro, status))
                                return onError(op, status);
                        }
                        TU_LOG_V << "loaded value " << value << " at instance address " << index;
                        currentCoro->pushData(value);
                        break;
                    }
                    case lyric_object::LOAD_ENUM: {
                        tempo_utils::Status status;
                        auto value = segmentManager->loadEnum(index, currentCoro, status);
                        if (value.type == DataCellType::INVALID) {
                            if (status.notOk())
                                return onError(op, status);
                            // invoke the allocator
                            auto *allocator = heapManager->prepareNew(
                                lyric_object::NEW_ENUM, index, status);
                            if (allocator == nullptr)
                                return onError(op, status);
                            status = allocator(this, m_state.get());
                            if (status.notOk())
                                return onError(op, status);
                            if (!subroutineManager->returnToCaller(currentCoro, status))
                                return onError(op, status);
                            // capture the uninitialized enum
                            value = currentCoro->peekData();
                            // push the ctor frame onto the call stack
                            std::vector<DataCell> noArguments;
                            if (!heapManager->constructNew(noArguments, status))
                                return onError(op, status);
                            // reenter the interpreter to invoke the enum ctor
                            auto initEnumResult = runSubinterpreter();
                            if (initEnumResult.isStatus())
                                return initEnumResult;
                            // store the enum
                            if (!segmentManager->storeEnum(index, value, currentCoro, status))
                                return onError(op, status);
                        }
                        TU_LOG_V << "loaded value " << value << " at enum address " << index;
                        currentCoro->pushData(value);
                        break;
                    }
                    default:
                        return onError(op, InterpreterStatus::forCondition(
                            InterpreterCondition::kInvalidOperandFlagsAddressV1, "unknown LOAD flags"));
                }
                break;
            }

            // pop value from the stack and store it in the current activation frame
            case lyric_object::Opcode::OP_STORE: {
                auto flags = op.operands.flags_u8_address_u32.flags;
                auto index = op.operands.flags_u8_address_u32.address;
                auto value = currentCoro->popData();
                auto &activation = currentCoro->peekCall();
                switch (flags) {
                    case lyric_object::STORE_ARGUMENT: {
                        activation.setArgument(index, value);
                        TU_LOG_V << "stored argument " << value;
                        break;
                    }
                    case lyric_object::STORE_LOCAL: {
                        activation.setLocal(index, value);
                        TU_LOG_V << "stored local " << value;
                        break;
                    }
                    case lyric_object::STORE_LEXICAL: {
                        activation.setLexical(index, value);
                        TU_LOG_V << "stored lexical " << value;
                        break;
                    }
                    case lyric_object::STORE_FIELD: {
                        InterpreterStatus status;
                        auto field = segmentManager->resolveDescriptor(currentCoro->peekSP(),
                            lyric_object::LinkageSection::Field, index, status);
                        if (!field.isValid())
                            return onError(op, status);
                        auto receiver = currentCoro->popData();
                        if (receiver.type != DataCellType::REF)
                            return onError(op, InterpreterStatus::forCondition(InterpreterCondition::kInvalidReceiver,
                                "invalid receiver for STORE"));
                        auto *instance = receiver.data.ref;
                        instance->setField(field, value);
                        TU_LOG_V << "stored value " << value << " in field " << field << " of receiver " << receiver;
                        break;
                    }
                    case lyric_object::STORE_STATIC: {
                        InterpreterStatus status;
                        if (!segmentManager->storeStatic(index, value, currentCoro, status))
                            return onError(op, status);
                        TU_LOG_V << "stored value " << value << " at static address " << index;
                        break;
                    }
                    default:
                        return onError(op, InterpreterStatus::forCondition(
                            InterpreterCondition::kInvalidOperandFlagsAddressV1, "unknown STORE flags"));
                }
                break;
            }

            // pop index from the stack and push variadic argument in the current activation onto the top of the stack
            case lyric_object::Opcode::OP_VA_LOAD: {
                auto arg = currentCoro->popData();
                const auto &activation = currentCoro->peekCall();
                if (arg.type != DataCellType::I64)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "LOAD index must be Integer"));
                if (arg.data.i64 < 0 || activation.numRest() <= arg.data.i64)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "LOAD index out of range"));
                currentCoro->pushData(activation.getRest(arg.data.i64));
                break;
            }

            // push count of the variadic arguments in the current activation onto the top of the stack
            case lyric_object::Opcode::OP_VA_SIZE: {
                const auto &activation = currentCoro->peekCall();
                currentCoro->pushData(DataCell(static_cast<int64_t>(activation.numRest())));
                break;
            }

            // pop value from the stack and discard it
            case lyric_object::Opcode::OP_POP: {
                currentCoro->dropData();
                break;
            }

            // duplicate top value on the stack and push it onto the top of the stack
            case lyric_object::Opcode::OP_DUP: {
                currentCoro->pushData(currentCoro->peekData());
                break;
            }

            // duplicate the picked value on the stack and push onto the top of the stack
            case lyric_object::Opcode::OP_PICK: {
                auto offset = op.operands.offset_u16.offset;
                currentCoro->pushData(currentCoro->peekData(offset));
                break;
            }

            // remove the value at the specified offset from the stack
            case lyric_object::Opcode::OP_DROP: {
                auto offset = op.operands.offset_u16.offset;
                currentCoro->dropData(offset);
                break;
            }

            // duplicate the picked value on the stack and push onto the top of the stack
            case lyric_object::Opcode::OP_RPICK: {
                auto offset = op.operands.offset_u16.offset;
                currentCoro->pushData(currentCoro->peekData(-1 - offset));
                break;
            }

            // remove the value at the specified offset from the stack
            case lyric_object::Opcode::OP_RDROP: {
                auto offset = op.operands.offset_u16.offset;
                currentCoro->dropData(-1 - offset);
                break;
            }

            // pop 2 integer values from the stack and add them, and push result onto the stack
            case lyric_object::Opcode::OP_I64_ADD: {
                auto rhs = currentCoro->popData();
                if (rhs.type != DataCellType::I64)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV2, "wrong type for rhs"));
                auto lhs = currentCoro->popData();
                if (lhs.type != DataCellType::I64)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for lhs"));
                DataCell result;
                result.type = DataCellType::I64;
                result.data.i64 = lhs.data.i64 + rhs.data.i64;
                currentCoro->pushData(result);
                break;
            }

            // pop 2 integer values from the stack and subtract them, and push result onto the stack
            case lyric_object::Opcode::OP_I64_SUB: {
                auto rhs = currentCoro->popData();
                if (rhs.type != DataCellType::I64)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV2, "wrong type for rhs"));
                auto lhs = currentCoro->popData();
                if (lhs.type != DataCellType::I64)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for lhs"));
                DataCell result;
                result.type = DataCellType::I64;
                result.data.i64 = lhs.data.i64 - rhs.data.i64;
                currentCoro->pushData(result);
                break;
            }

            // pop 2 integer values from the stack and multiply them, and push result onto the stack
            case lyric_object::Opcode::OP_I64_MUL: {
                auto rhs = currentCoro->popData();
                if (rhs.type != DataCellType::I64)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV2, "wrong type for rhs"));
                auto lhs = currentCoro->popData();
                if (lhs.type != DataCellType::I64)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for lhs"));
                DataCell result;
                result.type = DataCellType::I64;
                result.data.i64 = lhs.data.i64 * rhs.data.i64;
                currentCoro->pushData(result);
                break;
            }

            // pop 2 integer values from the stack and divide them, and push result onto the stack
            case lyric_object::Opcode::OP_I64_DIV: {
                auto rhs = currentCoro->popData();
                if (rhs.type != DataCellType::I64)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV2, "wrong type for rhs"));
                auto lhs = currentCoro->popData();
                if (lhs.type != DataCellType::I64)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for lhs"));
                DataCell result;
                result.type = DataCellType::I64;
                result.data.i64 = lhs.data.i64 / rhs.data.i64;
                currentCoro->pushData(result);
                break;
            }

            // pop 1 integer value from the stack and negate it, and push result onto the stack
            case lyric_object::Opcode::OP_I64_NEG: {
                auto rhs = currentCoro->popData();
                if (rhs.type != DataCellType::I64)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for rhs"));
                DataCell result;
                result.type = DataCellType::I64;
                result.data.i64 = -rhs.data.i64;
                currentCoro->pushData(result);
                break;
            }

            // pop 2 float values from the stack and add them, and push result onto the stack
            case lyric_object::Opcode::OP_DBL_ADD: {
                auto rhs = currentCoro->popData();
                if (rhs.type != DataCellType::DBL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV2, "wrong type for rhs"));
                auto lhs = currentCoro->popData();
                if (lhs.type != DataCellType::DBL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for lhs"));
                DataCell result;
                result.type = DataCellType::DBL;
                result.data.dbl = lhs.data.dbl + rhs.data.dbl;
                currentCoro->pushData(result);
                break;
            }

            // pop 2 float values from the stack and subtract them, and push result onto the stack
            case lyric_object::Opcode::OP_DBL_SUB: {
                auto rhs = currentCoro->popData();
                if (rhs.type != DataCellType::DBL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV2, "wrong type for rhs"));
                auto lhs = currentCoro->popData();
                if (lhs.type != DataCellType::DBL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for lhs"));
                DataCell result;
                result.type = DataCellType::DBL;
                result.data.dbl = lhs.data.dbl - rhs.data.dbl;
                currentCoro->pushData(result);
                break;
            }

            // pop 2 float values from the stack and multiply them, and push result onto the stack
            case lyric_object::Opcode::OP_DBL_MUL: {
                auto rhs = currentCoro->popData();
                if (rhs.type != DataCellType::DBL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV2, "wrong type for rhs"));
                auto lhs = currentCoro->popData();
                if (lhs.type != DataCellType::DBL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for lhs"));
                DataCell result;
                result.type = DataCellType::DBL;
                result.data.dbl = lhs.data.dbl * rhs.data.dbl;
                currentCoro->pushData(result);
                break;
            }

            // pop 2 float values from the stack and divide them, and push result onto the stack
            case lyric_object::Opcode::OP_DBL_DIV: {
                auto rhs = currentCoro->popData();
                if (rhs.type != DataCellType::DBL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV2, "wrong type for rhs"));
                auto lhs = currentCoro->popData();
                if (lhs.type != DataCellType::DBL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for lhs"));
                DataCell result;
                result.type = DataCellType::DBL;
                result.data.dbl = lhs.data.dbl / rhs.data.dbl;
                currentCoro->pushData(result);
                break;
            }

            // pop 1 float value from the stack and negate it, and push result onto the stack
            case lyric_object::Opcode::OP_DBL_NEG: {
                auto rhs = currentCoro->popData();
                if (rhs.type != DataCellType::DBL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for rhs"));
                DataCell result;
                result.type = DataCellType::DBL;
                result.data.dbl = -rhs.data.dbl;
                currentCoro->pushData(result);
                break;
            }

            // pop 2 bool values from the stack and compare them, and push result onto the stack
            case lyric_object::Opcode::OP_BOOL_CMP: {
                auto rhs = currentCoro->popData();
                if (rhs.type != DataCellType::BOOL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV2, "wrong type for rhs"));
                auto lhs = currentCoro->popData();
                if (lhs.type != DataCellType::BOOL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for lhs"));
                DataCell result;
                result.type = DataCellType::I64;
                if (lhs.data.b < rhs.data.b)
                    result.data.i64 = -1;
                else if (lhs.data.b > rhs.data.b)
                    result.data.i64 = 1;
                else
                    result.data.i64 = 0;
                currentCoro->pushData(result);
                break;
            }

            // pop 2 integer values from the stack and compare them, and push result onto the stack
            case lyric_object::Opcode::OP_I64_CMP: {
                auto rhs = currentCoro->popData();
                if (rhs.type != DataCellType::I64)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV2, "wrong type for rhs"));
                auto lhs = currentCoro->popData();
                if (lhs.type != DataCellType::I64)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for lhs"));
                DataCell result;
                result.type = DataCellType::I64;
                if (lhs.data.i64 < rhs.data.i64)
                    result.data.i64 = -1;
                else if (lhs.data.i64 > rhs.data.i64)
                    result.data.i64 = 1;
                else
                    result.data.i64 = 0;
                currentCoro->pushData(result);
                break;
            }

            // pop 2 float values from the stack and compare them, and push result onto the stack
            case lyric_object::Opcode::OP_DBL_CMP: {
                auto rhs = currentCoro->popData();
                if (rhs.type != DataCellType::DBL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV2, "wrong type for rhs"));
                auto lhs = currentCoro->popData();
                if (lhs.type != DataCellType::DBL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for lhs"));
                DataCell result;
                result.type = DataCellType::I64;
                if (lhs.data.dbl < rhs.data.dbl)
                    result.data.i64 = -1;
                else if (lhs.data.dbl > rhs.data.dbl)
                    result.data.i64 = 1;
                else
                    result.data.i64 = 0;
                currentCoro->pushData(result);
                break;
            }

            // pop 2 chr values from the stack and compare them, and push result onto the stack
            case lyric_object::Opcode::OP_CHR_CMP: {
                auto rhs = currentCoro->popData();
                if (rhs.type != DataCellType::CHAR32)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV2, "wrong type for rhs"));
                auto lhs = currentCoro->popData();
                if (lhs.type != DataCellType::CHAR32)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for lhs"));
                DataCell result;
                result.type = DataCellType::I64;
                if (lhs.data.chr < rhs.data.chr)
                    result.data.i64 = -1;
                else if (lhs.data.chr > rhs.data.chr)
                    result.data.i64 = 1;
                else
                    result.data.i64 = 0;
                currentCoro->pushData(result);
                break;
            }

            // pop two type descriptors from the stack and compare them, and push result onto the stack
            case lyric_object::Opcode::OP_TYPE_CMP: {
                auto rhs = currentCoro->popData();
                if (rhs.type != DataCellType::TYPE)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV2, "wrong type for rhs"));
                auto lhs = currentCoro->popData();
                if (lhs.type != DataCellType::TYPE)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for lhs"));
                auto compareTypesResult = typeManager->compareTypes(lhs, rhs);
                if (compareTypesResult.isStatus())
                    return onError(op, compareTypesResult.getStatus());
                DataCell result;
                result.type = DataCellType::I64;
                switch (compareTypesResult.getResult()) {
                    case TypeComparison::EXTENDS:
                        result.data.i64 = -1;
                        break;
                    case TypeComparison::EQUAL:
                        result.data.i64 = 0;
                        break;
                    case TypeComparison::SUPER:
                    case TypeComparison::DISJOINT:
                        result.data.i64 = 1;
                        break;
                }
                currentCoro->pushData(result);
                break;
            }

            // pop 2 bool values from the stack and perform logical AND, and push result onto the stack
            case lyric_object::Opcode::OP_LOGICAL_AND: {
                auto rhs = currentCoro->popData();
                if (rhs.type != DataCellType::BOOL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV2, "wrong type for rhs"));
                auto lhs = currentCoro->popData();
                if (lhs.type != DataCellType::BOOL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for lhs"));
                DataCell result;
                result.type = DataCellType::BOOL;
                result.data.b = lhs.data.b && rhs.data.b;
                currentCoro->pushData(result);
                break;
            }

            // pop 2 bool values from the stack and perform logical OR, and push result onto the stack
            case lyric_object::Opcode::OP_LOGICAL_OR: {
                auto rhs = currentCoro->popData();
                if (rhs.type != DataCellType::BOOL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV2, "wrong type for rhs"));
                auto lhs = currentCoro->popData();
                if (lhs.type != DataCellType::BOOL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for lhs"));
                DataCell result;
                result.type = DataCellType::BOOL;
                result.data.b = lhs.data.b || rhs.data.b;
                currentCoro->pushData(result);
                break;
            }

            // pop 1 bool value from the stack and perform logical NOT, and push result onto the stack
            case lyric_object::Opcode::OP_LOGICAL_NOT: {
                auto rhs = currentCoro->popData();
                if (rhs.type != DataCellType::BOOL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "wrong type for rhs"));
                DataCell result;
                result.type = DataCellType::BOOL;
                result.data.b = !rhs.data.b;
                currentCoro->pushData(result);
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
                auto cmp = currentCoro->popData();
                if (cmp.type == DataCellType::NIL) {
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
                auto cmp = currentCoro->popData();
                if (cmp.type != DataCellType::NIL) {
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
                auto cmp = currentCoro->popData();
                if (cmp.type != DataCellType::BOOL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "value must be a boolean"));
                if (cmp.data.b) {
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
                auto cmp = currentCoro->popData();
                if (cmp.type != DataCellType::BOOL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "value must be a boolean"));
                if (!cmp.data.b) {
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
                auto cmp = currentCoro->popData();
                if (cmp.type != DataCellType::I64)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "value must be an integer"));
                if (cmp.data.i64 == 0) {
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
                auto cmp = currentCoro->popData();
                if (cmp.type != DataCellType::I64)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "value must be an integer"));
                if (cmp.data.i64 != 0) {
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
                auto cmp = currentCoro->popData();
                if (cmp.type != DataCellType::I64)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "value must be an integer"));
                if (cmp.data.i64 < 0) {
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
                auto cmp = currentCoro->popData();
                if (cmp.type != DataCellType::I64)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "value must be an integer"));
                if (cmp.data.i64 <= 0) {
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
                auto cmp = currentCoro->popData();
                if (cmp.type != DataCellType::I64)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "value must be an integer"));
                if (cmp.data.i64 > 0) {
                    auto delta = op.operands.jump_i16.jump;
                    if (!currentCoro->moveIP(delta))
                        return onError(op, InterpreterStatus::forCondition(
                            InterpreterCondition::kInvalidOperandJumpV1, "invalid jump offset"));
                    TU_LOG_V << "moved ip " << delta << " bytes to " << currentCoro->peekIP();
                }
                break;
            }

            // pop value from stack, and jump to offset if value is not zero
            case lyric_object::Opcode::OP_IF_GE: {
                auto cmp = currentCoro->popData();
                if (cmp.type != DataCellType::I64)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "value must be an integer"));
                if (cmp.data.i64 >= 0) {
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
                auto placement = currentCoro->popData(placementSize);

                // construct the activation call frame
                InterpreterStatus status;
                if (!subroutineManager->callStatic(address, placement, currentCoro, status))
                    return onError(op, status);
                break;
            }

            // execute the method specified by index into the vtable of the object on the top of the stack
            case lyric_object::Opcode::OP_CALL_VIRTUAL: {
                auto callAddress = op.operands.flags_u8_address_u32_placement_u16.address;
                auto placementSize = op.operands.flags_u8_address_u32_placement_u16.placement;
                auto flags = op.operands.flags_u8_address_u32_placement_u16.flags;
                DataCell receiver;
                std::vector<DataCell> placement;

                if (flags & lyric_object::CALL_RECEIVER_FOLLOWS) {
                    // receiver comes after arguments so we pop receiver first
                    receiver = currentCoro->popData();
                    if (receiver.type != DataCellType::REF)
                        return onError(op,
                            InterpreterStatus::forCondition(
                                InterpreterCondition::kInvalidReceiver, "invalid receiver for virtual call"));
                    placement = currentCoro->popData(placementSize);
                } else {
                    // receiver comes before arguments so we pop placement first
                    placement = currentCoro->popData(placementSize);
                    receiver = currentCoro->popData();
                    if (receiver.type != DataCellType::REF)
                        return onError(op,
                            InterpreterStatus::forCondition(
                                InterpreterCondition::kInvalidReceiver, "invalid receiver for virtual call"));
                }

                if (flags & lyric_object::CALL_FORWARD_REST) {
                    auto &call = currentCoro->peekCall();
                    for (int i = 0; i < call.numRest(); i++) {
                        placement.push_back(call.getRest(i));
                    }
                }

                // construct the activation call frame
                InterpreterStatus status;
                if (!subroutineManager->callVirtual(receiver, callAddress, placement, currentCoro, status))
                    return onError(op, status);
                break;
            }

            case lyric_object::Opcode::OP_CALL_CONCEPT: {
                auto actionAddress = op.operands.flags_u8_address_u32_placement_u16.address;
                auto placementSize = op.operands.flags_u8_address_u32_placement_u16.placement;
                auto flags = op.operands.flags_u8_address_u32_placement_u16.flags;
                DataCell receiver;
                std::vector<DataCell> placement;

                auto conceptDescriptor = currentCoro->popData();
                if (conceptDescriptor.type != DataCellType::CONCEPT)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "invalid descriptor for concept call"));

                if (flags & lyric_object::CALL_RECEIVER_FOLLOWS) {
                    // receiver comes after arguments so we pop receiver first
                    receiver = currentCoro->popData();
                    if (receiver.type != DataCellType::REF)
                        return onError(op, InterpreterStatus::forCondition(
                            InterpreterCondition::kInvalidReceiver, "invalid receiver for concept call"));
                    placement = currentCoro->popData(placementSize);
                } else {
                    // receiver comes before arguments so we pop placement first
                    placement = currentCoro->popData(placementSize);
                    receiver = currentCoro->popData();
                    if (receiver.type != DataCellType::REF)
                        return onError(op, InterpreterStatus::forCondition(
                            InterpreterCondition::kInvalidReceiver, "invalid receiver for concept call"));
                }

                // construct the activation call frame
                InterpreterStatus status;
                if (!subroutineManager->callConcept(
                    receiver, conceptDescriptor, actionAddress, placement, currentCoro, status))
                    return onError(op, status);
                break;
            }

            case lyric_object::Opcode::OP_CALL_EXISTENTIAL: {
                auto callAddress = op.operands.flags_u8_address_u32_placement_u16.address;
                auto placementSize = op.operands.flags_u8_address_u32_placement_u16.placement;

                auto existentialDescriptor = currentCoro->popData();
                if (existentialDescriptor.type != DataCellType::EXISTENTIAL)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kInvalidDataStackV1, "invalid descriptor for existential call"));

                auto placement = currentCoro->popData(placementSize);
                auto receiver = currentCoro->popData();

                // construct the activation call frame
                InterpreterStatus status;
                if (!subroutineManager->callExistential(
                    receiver, existentialDescriptor, callAddress, placement, currentCoro, status))
                    return onError(op, status);
                break;
            }

            // return from the current activation
            case lyric_object::Opcode::OP_RETURN: {
                // if we reached the call stack guard then pop the guard
                bool reachedGuard = currentCoro->peekGuard() == currentCoro->callStackSize();
                if (reachedGuard)
                    currentCoro->popGuard();
                InterpreterStatus status;
                // check if we reached the bottom of the call stack
                if (!subroutineManager->returnToCaller(currentCoro, status)) {
                    if (status.notOk())
                        return onError(op, status);
                    // if we're executing the main task and we have no return address then halt
                    if (systemScheduler->currentTask()->getTaskType() == TaskType::Main)
                        return onHalt(op);
                    // otherwise we're executing a worker task, so we do nothing
                    break;
                }
                // if the call stack is still valid and we reached a guard then return from the subinterpreter
                if (reachedGuard) {
                    if (currentCoro->dataStackSize() > 0)
                        return currentCoro->popData();
                    return DataCell();
                }
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
                    auto index = currentCoro->popData();
                    if (index.type != DataCellType::I64)
                        return onError(op, InterpreterStatus::forCondition(
                            InterpreterCondition::kInvalidDataStackV1, "invalid trap index"));
                    if (index.data.i64 < 0 || std::numeric_limits<uint32_t>::max() < index.data.i64)
                        return onError(op, InterpreterStatus::forCondition(
                            InterpreterCondition::kInvalidDataStackV1, "invalid trap index"));
                    address = index.data.i64;
                }
                TU_LOG_V << "trap index is " << address;
                auto *sp = currentCoro->peekSP();
                auto *trap = sp->getTrap(address);
                if (trap == nullptr)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "no trap found"));
                auto status = trap(this, m_state.get());  // invoke the trap
                if (!status.isOk())
                    return onError(op, status);
                // ensure currentCoro is up to date
                currentCoro = m_state->currentCoro();
                break;
            }

            // invoke the constructor specified by the static address operand
            case lyric_object::Opcode::OP_NEW: {
                auto flags = op.operands.flags_u8_address_u32_placement_u16.flags;
                auto address = op.operands.flags_u8_address_u32_placement_u16.address;
                auto placementSize = op.operands.flags_u8_address_u32_placement_u16.placement;
                auto placement = currentCoro->popData(placementSize);

                // invoke the allocator
                tempo_utils::Status status;
                auto *allocator = heapManager->prepareNew(
                    lyric_object::GET_NEW_TYPE(flags), address, status);
                if (allocator == nullptr)
                    return onError(op, status);
                status = allocator(this, m_state.get());
                if (status.notOk())
                    return onError(op, status);
                if (!subroutineManager->returnToCaller(currentCoro, status))
                    return onError(op, status);

                auto callFlags = lyric_object::GET_CALL_FLAGS(flags);

                if (callFlags & lyric_object::CALL_FORWARD_REST) {
                    auto &call = currentCoro->peekCall();
                    for (int i = 0; i < call.numRest(); i++) {
                        placement.push_back(call.getRest(i));
                    }
                }

                // invoke the constructor
                if (!heapManager->constructNew(placement, status))
                    return onError(op, status);
                break;
            }

            case lyric_object::Opcode::OP_TYPE_OF: {
                auto typeOfResult = typeManager->typeOf(currentCoro->popData());
                if (typeOfResult.isStatus())
                    return onError(op, typeOfResult.getStatus());
                auto typeOf = typeOfResult.getResult();
                if (typeOf.type != DataCellType::TYPE)
                    return onError(op, InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid type result"));
                currentCoro->pushData(typeOf);
                break;
            }

            // interrupt the interpreter.  if there is a value on the stack, return it, otherwise return nil.
            case lyric_object::Opcode::OP_INTERRUPT: {
                auto status = onInterrupt(currentCoro->popData());
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
    auto result = currentCoro->popData();
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
lyric_runtime::BytecodeInterpreter::onInterrupt(const DataCell &cell)
{
    TU_LOG_VV << "interrupting interpreter";
    if (m_inspector)
        return m_inspector->onInterrupt(cell, this, m_state.get());
    return {};
}

tempo_utils::Result<lyric_runtime::DataCell>
lyric_runtime::BytecodeInterpreter::onError(const lyric_object::OpCell &op, const tempo_utils::Status &status)
{
    TU_LOG_VV << status;
    if (m_inspector)
        return m_inspector->onError(op, status, this, m_state.get());
    return status;
}

tempo_utils::Result<lyric_runtime::DataCell>
lyric_runtime::BytecodeInterpreter::onHalt(const lyric_object::OpCell &op)
{
    TU_LOG_VV << "halting interpreter";

    auto *currentCoro = m_state->currentCoro();
    auto mainReturn = currentCoro->dataStackSize() > 0? currentCoro->popData() : DataCell{};

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
