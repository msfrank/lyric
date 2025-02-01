#ifndef LYRIC_OPTIMIZER_PARSE_PROC_H
#define LYRIC_OPTIMIZER_PARSE_PROC_H

#include <lyric_assembler/proc_handle.h>

#include "control_flow_graph.h"

namespace lyric_optimizer {

    tempo_utils::Result<ControlFlowGraph> parse_proc(const lyric_assembler::ProcHandle *procHandle);
}

#endif // LYRIC_OPTIMIZER_PARSE_PROC_H
