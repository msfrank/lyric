
#include <lyric_runtime/call_cell.h>
#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_test/test_inspector.h>
#include <tempo_utils/log_console.h>
#include <tempo_utils/log_stream.h>

lyric_test::TestInspector::TestInspector()
{
}

tempo_utils::Status
lyric_test::TestInspector::beforeOp(
    const lyric_object::OpCell &op,
    lyric_runtime::InterpreterState *state)
{
    TU_CONSOLE_OUT << "exec opcode " << op.opcode << " (" << (uint8_t) op.opcode << ")";
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
lyric_test::TestInspector::afterOp(
    const lyric_object::OpCell &op,
    lyric_runtime::InterpreterState *state)
{
    printDataStack(state);
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
lyric_test::TestInspector::onInterrupt(
    const lyric_runtime::DataCell &cell,
    lyric_runtime::InterpreterState *state)
{
    return lyric_runtime::InterpreterStatus::forCondition(
        lyric_runtime::InterpreterCondition::kInterrupted, cell.toString());
}

tempo_utils::Result<lyric_runtime::DataCell>
lyric_test::TestInspector::onError(
    const lyric_object::OpCell &op,
    const tempo_utils::Status &status,
    lyric_runtime::InterpreterState *state)
{
    return status;
}

tempo_utils::Result<lyric_runtime::DataCell>
lyric_test::TestInspector::onHalt(
    const lyric_object::OpCell &op,
    const lyric_runtime::DataCell &cell,
    lyric_runtime::InterpreterState *state)
{
    return cell;
}

lyric_common::SymbolUrl
frame_to_symbol_url(const lyric_runtime::CallCell &frame, lyric_runtime::InterpreterState *state)
{
    auto *segment = state->segmentManager()->getSegment(frame.getCallSegment());
    if (segment == nullptr)
        return lyric_common::SymbolUrl();
    auto location = segment->getLocation();
    auto object = segment->getObject().getObject();
    auto call = object.getCall(frame.getCallIndex());
    if (!call.isValid())
        return lyric_common::SymbolUrl();
    auto path = call.getSymbolPath();
    return lyric_common::SymbolUrl(location, path);
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
        TU_LOG_INFO << "Frame #" << frameNr << ": " << frameDescription;
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
        int lowerGuard = currentCoro->peekCall(untilFrameNr).getStackGuard();
        int currFrameNr = currentCoro->callStackSize() - 1;
        int currGuard = currentCoro->peekCall().getStackGuard();
        int valueNr = currentCoro->dataStackSize() - 1;

        auto frameDescription = frame_to_description(currentCoro->peekCall(), state);
        TU_CONSOLE_OUT << "Frame #" << currFrameNr << ": " << frameDescription;

        for (auto iterator = currentCoro->dataBegin(); iterator != currentCoro->dataEnd(); iterator++) {
            if (valueNr < lowerGuard)
                break;
            if (valueNr < currGuard) {
                while (valueNr < currGuard) {
                    currFrameNr--;
                    const auto &frame = currentCoro->peekCall(currFrameNr);
                    currGuard = frame.getStackGuard();
                    frameDescription = frame_to_description(frame, state);
                    TU_CONSOLE_OUT << "Frame #" << currFrameNr << ": " << frameDescription;
                }
            }
            TU_CONSOLE_OUT << absl::Dec(valueNr, absl::kSpacePad3) << "| " << iterator->toString();
            valueNr--;
        }

        while (currFrameNr > 0) {
            currFrameNr--;
            const auto &frame = currentCoro->peekCall(currFrameNr);
            frameDescription = frame_to_description(frame, state);
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

    const auto &frame = currentCoro->peekCall(frameNr);
    int lowerGuard = frame.getStackGuard();
    int upperGuard = std::numeric_limits<int>::max();
    if (frameNr + 1 < currentCoro->callStackSize())
        upperGuard = currentCoro->peekCall(frameNr + 1).getStackGuard();

    TU_CONSOLE_OUT << "-------- DATA STACK DUMP --------";

    auto frameDescription = frame_to_description(currentCoro->peekCall(), state);
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