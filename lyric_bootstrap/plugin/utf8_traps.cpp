//#include <unicode/ustring.h>
//
//#include <lyric_runtime/interpreter_state.h>
//
//#include "utf8_traps.h"
//
//lyric_runtime::InterpreterStatus
//utf8_at(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
//{
//    auto *currentCoro = state->currentCoro();
//
//    auto &frame = currentCoro->peekCall();
//
//    TU_ASSERT(frame.numArguments() == 2);
//    const auto &s = frame.getArgument(0);
//    TU_ASSERT(s.type == lyric_runtime::DataCellType::UTF8);
//    const auto &i = frame.getArgument(1);
//    TU_ASSERT(s.type == lyric_runtime::DataCellType::I64);
//
//    UChar32 chr;
//    U8_GET(s.data.utf8.data, 0, i.data.i64, s.data.utf8.size, chr);
//    currentCoro->pushData(lyric_runtime::DataCell(chr));
//    return lyric_runtime::InterpreterStatus::ok();
//}
//
//lyric_runtime::InterpreterStatus
//utf8_compare(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
//{
//    auto *currentCoro = state->currentCoro();
//
//    auto &frame = currentCoro->peekCall();
//
//    TU_ASSERT(frame.numArguments() == 2);
//    const auto &lhs = frame.getArgument(0);
//    TU_ASSERT(lhs.type == lyric_runtime::DataCellType::UTF8);
//    const auto &rhs = frame.getArgument(1);
//    TU_ASSERT(rhs.type == lyric_runtime::DataCellType::UTF8);
//
//    auto cmp = u_strCompare(lhs.data.utf8.data,
//                            lhs.data.utf8.size,
//                            rhs.data.utf8.data,
//                            rhs.data.utf8.size,
//                            true);
//    currentCoro->pushData(lyric_runtime::DataCell(static_cast<int64_t>(cmp)));
//    return lyric_runtime::InterpreterStatus::ok();
//}
//
//lyric_runtime::InterpreterStatus
//utf8_size(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
//{
//    auto *currentCoro = state->currentCoro();
//
//    auto &frame = currentCoro->peekCall();
//
//    TU_ASSERT(frame.numArguments() == 1);
//    const auto &cell = frame.getArgument(0);
//    TU_ASSERT(cell.type == lyric_runtime::DataCellType::UTF8);
//
//    auto count = u_countChar32(cell.data.utf8.data, cell.data.utf8.size);
//    currentCoro->pushData(lyric_runtime::DataCell(static_cast<int64_t>(count)));
//    return lyric_runtime::InterpreterStatus::ok();
//}
