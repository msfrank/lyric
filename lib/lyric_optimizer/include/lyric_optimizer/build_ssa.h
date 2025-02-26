#ifndef LYRIC_OPTIMIZER_BUILD_SSA_H
#define LYRIC_OPTIMIZER_BUILD_SSA_H

#include "control_flow_graph.h"
#include "optimizer_result.h"

namespace lyric_optimizer {

    tempo_utils::Status build_ssa(const ControlFlowGraph &cfg);

}

#endif // LYRIC_OPTIMIZER_BUILD_SSA_H
