
#include <lyric_runtime/call_cell.h>
#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_test/test_inspector.h>
#include <tempo_utils/log_console.h>
#include <tempo_utils/log_stream.h>

lyric_test::TestInspector::TestInspector()
    : m_currentTask(nullptr)
{
}

tempo_utils::Status
lyric_test::TestInspector::beforeOp(
    const lyric_object::OpCell &op,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    auto *currentTask = state->systemScheduler()->currentTask();

    if (currentTask != m_currentTask) {
        if (m_currentTask != nullptr) {
            TU_CONSOLE_OUT << "*** task switch from " << m_currentTask << " to " << currentTask << " ***";
            printCallStack(state);
            TU_CONSOLE_OUT << "";
        }
        m_currentTask = currentTask;
    }

    TU_CONSOLE_OUT << "task " << m_currentTask << ": exec opcode " << op.opcode
        << " (" << (uint8_t) op.opcode << ")"
        << " rd:" << interp->getRecursionDepth()
        << " sc:" << interp->getSliceCounter()
        << " ic:" << interp->getInstructionCounter();
    return {};
}

tempo_utils::Status
lyric_test::TestInspector::afterOp(
    const lyric_object::OpCell &op,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    printDataStack(state);
    TU_CONSOLE_OUT << "";
    return {};
}

tempo_utils::Status
lyric_test::TestInspector::onInterrupt(
    const lyric_runtime::DataCell &cell,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    return lyric_runtime::InterpreterStatus::forCondition(
        lyric_runtime::InterpreterCondition::kInterrupted, cell.toString());
}

tempo_utils::Result<lyric_runtime::DataCell>
lyric_test::TestInspector::onError(
    const lyric_object::OpCell &op,
    const tempo_utils::Status &status,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    return status;
}

tempo_utils::Result<lyric_runtime::DataCell>
lyric_test::TestInspector::onHalt(
    const lyric_object::OpCell &op,
    const lyric_runtime::DataCell &cell,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    return cell;
}

lyric_common::SymbolUrl
frame_to_symbol_url(const lyric_runtime::CallCell &frame, lyric_runtime::InterpreterState *state)
{
    auto *segment = state->segmentManager()->getSegment(frame.getCallSegment());
    if (segment == nullptr)
        return {};
    auto objectLocation = segment->getObjectLocation();
    auto object = segment->getObject();
    auto call = object.getCall(frame.getCallIndex());
    if (!call.isValid())
        return {};
    auto symbolPath = call.getSymbolPath();
    return lyric_common::SymbolUrl(objectLocation, symbolPath);
}

std::string
frame_to_description(const lyric_runtime::CallCell &frame, lyric_runtime::InterpreterState *state)
{
    return frame_to_symbol_url(frame, state).toString();
}

void
lyric_test::TestInspector::printCallStack(lyric_runtime::InterpreterState *state) const
{
    TU_CONSOLE_OUT << "-------- CALL STACK DUMP --------";
    auto *currentCoro = state->currentCoro();
    int frameNr = currentCoro->callStackSize() - 1;
    for (auto iterator = currentCoro->callsBegin(); iterator != currentCoro->callsEnd(); iterator++) {
        auto frameDescription = frame_to_description(*iterator, state);
        TU_CONSOLE_OUT << "Frame #" << frameNr << ": " << frameDescription;
        frameNr--;
    }
    TU_CONSOLE_OUT << "---------------------------------";
}

void
lyric_test::TestInspector::printDataStack(lyric_runtime::InterpreterState *state, int untilFrameNr) const
{
    auto *currentCoro = state->currentCoro();
    if (!currentCoro)
        return;

    if (untilFrameNr < 0 || untilFrameNr > currentCoro->callStackSize() - 1)
        return;

    TU_CONSOLE_OUT << "-------- DATA STACK DUMP --------";

    if (currentCoro->callStackSize() > 0) {
        lyric_runtime::CallCell *lowerCall, *currCall;
        TU_RAISE_IF_NOT_OK (currentCoro->peekCall(&lowerCall, untilFrameNr));
        int lowerGuard = lowerCall->getStackGuard();
        int currFrameNr = currentCoro->callStackSize() - 1;
        TU_RAISE_IF_NOT_OK (currentCoro->peekCall(&currCall));
        int currGuard = currCall->getStackGuard();
        int valueNr = currentCoro->dataStackSize() - 1;

        auto frameDescription = frame_to_description(*currCall, state);
        TU_CONSOLE_OUT << "Frame #" << currFrameNr << ": " << frameDescription;

        for (auto iterator = currentCoro->dataBegin(); iterator != currentCoro->dataEnd(); iterator++) {
            if (valueNr < lowerGuard)
                break;
            if (valueNr < currGuard) {
                while (valueNr < currGuard) {
                    currFrameNr--;
                    TU_RAISE_IF_NOT_OK (currentCoro->peekCall(&currCall, currFrameNr));
                    currGuard = currCall->getStackGuard();
                    frameDescription = frame_to_description(*currCall, state);
                    TU_CONSOLE_OUT << "Frame #" << currFrameNr << ": " << frameDescription;
                }
            }
            TU_CONSOLE_OUT << absl::Dec(valueNr, absl::kSpacePad3) << "| " << iterator->toString();
            valueNr--;
        }

        while (currFrameNr > 0) {
            currFrameNr--;
            TU_RAISE_IF_NOT_OK (currentCoro->peekCall(&currCall, currFrameNr));
            frameDescription = frame_to_description(*currCall, state);
            TU_CONSOLE_OUT << "Frame #" << currFrameNr << ": " << frameDescription;
        }
    }

    TU_CONSOLE_OUT << "---------------------------------";
}

void
lyric_test::TestInspector::printDataStackForFrame(lyric_runtime::InterpreterState *state, int frameNr) const
{
    auto *currentCoro = state->currentCoro();

    if (frameNr < 0 || frameNr > currentCoro->callStackSize() - 1)
        return;

    lyric_runtime::CallCell *lowerCall;
    TU_RAISE_IF_NOT_OK (currentCoro->peekCall(&lowerCall, frameNr));
    int lowerGuard = lowerCall->getStackGuard();
    int upperGuard = std::numeric_limits<int>::max();
    if (frameNr + 1 < currentCoro->callStackSize()) {
        lyric_runtime::CallCell *upperCall;
        currentCoro->peekCall(&upperCall, frameNr + 1);
        upperGuard = upperCall->getStackGuard();
    }

    TU_CONSOLE_OUT << "-------- DATA STACK DUMP --------";

    lyric_runtime::CallCell *currCall;
    TU_RAISE_IF_NOT_OK (currentCoro->peekCall(&currCall));
    auto frameDescription = frame_to_description(*currCall, state);
    TU_CONSOLE_OUT << "Frame #" << frameNr << ": " << frameDescription;

    int cellNr = currentCoro->dataStackSize() - 1;
    for (auto iterator = currentCoro->dataBegin(); iterator != currentCoro->dataEnd(); iterator++) {
        if (cellNr > upperGuard)
            continue;
        if (cellNr <= lowerGuard)
            break;
        TU_CONSOLE_OUT << absl::Dec(cellNr--, absl::kSpacePad3) << "| " << iterator->toString();
    }

    TU_CONSOLE_OUT << "---------------------------------";
}