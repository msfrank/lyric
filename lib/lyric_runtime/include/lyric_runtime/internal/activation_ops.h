#ifndef LYRIC_RUNTIME_INTERNAL_ACTIVATION_OPS_H
#define LYRIC_RUNTIME_INTERNAL_ACTIVATION_OPS_H

#include "../bytecode_interpreter.h"
#include "../heap_manager.h"
#include "../interpreter_state.h"
#include "../segment_manager.h"
#include "../stackful_coroutine.h"
#include "../subroutine_manager.h"

namespace lyric_runtime::internal {

    tempo_utils::Status load(
        StackfulCoroutine *currentCoro,
        SegmentManager *segmentManager,
        SubroutineManager *subroutineManager,
        HeapManager *heapManager,
        InterpreterState *interpreterState,
        BytecodeInterpreter *interpreter,
        tu_uint32 address,
        tu_uint8 flags);

    tempo_utils::Status store(
        StackfulCoroutine *currentCoro,
        SegmentManager *segmentManager,
        tu_uint32 address,
        tu_uint8 flags);
}

#endif // LYRIC_RUNTIME_INTERNAL_ACTIVATION_OPS_H
