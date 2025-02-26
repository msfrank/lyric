#ifndef LYRIC_OPTIMIZER_BUILD_PROC_H
#define LYRIC_OPTIMIZER_BUILD_PROC_H

#include "control_flow_graph.h"
#include "optimizer_result.h"

namespace lyric_optimizer {

    tempo_utils::Status build_proc(const ControlFlowGraph &cfg, lyric_assembler::ProcHandle *procHandle);

}

#endif // LYRIC_OPTIMIZER_BUILD_PROC_H
