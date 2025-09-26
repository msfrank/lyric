#ifndef LYRIC_RUNTIME_INTERNAL_CONSTRUCT_INSTANCE_H
#define LYRIC_RUNTIME_INTERNAL_CONSTRUCT_INSTANCE_H

#include <lyric_runtime/bytecode_segment.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/segment_manager.h>
#include <lyric_runtime/stackful_coroutine.h>

namespace lyric_runtime::internal {

    tempo_utils::Status construct_instance(
        tu_uint32 address,
        tu_uint8 flags,
        StackfulCoroutine *currentCoro,
        SegmentManager *segmentManager,
        BytecodeInterpreter *interp,
        InterpreterState *state);
}

#endif // LYRIC_RUNTIME_INTERNAL_CONSTRUCT_INSTANCE_H
