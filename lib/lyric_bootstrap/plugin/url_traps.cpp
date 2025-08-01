
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/url_ref.h>
#include <tempo_utils/log_stream.h>

#include "url_traps.h"

tempo_utils::Status
url_equals(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.type == lyric_runtime::DataCellType::URL);
    const auto &arg1 = frame.getArgument(1);
    TU_ASSERT(arg1.type == lyric_runtime::DataCellType::URL);

    auto *lhs = arg0.data.url;
    auto *rhs = arg1.data.url;
    currentCoro->pushData(lhs->uriEquals(rhs));
    return {};
}