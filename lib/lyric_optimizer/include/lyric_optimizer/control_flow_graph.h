#ifndef LYRIC_OPTIMIZER_CONTROL_FLOW_GRAPH_H
#define LYRIC_OPTIMIZER_CONTROL_FLOW_GRAPH_H

#include <lyric_assembler/proc_handle.h>
#include <lyric_common/symbol_url.h>

#include "basic_block.h"

namespace lyric_optimizer {

    // forward declarations
    namespace internal {
        struct GraphPriv;
    }

    class ControlFlowGraph {
    public:
        ControlFlowGraph();
        ControlFlowGraph(const ControlFlowGraph &other);

        BasicBlock getEntryBlock() const;
        BasicBlock getExitBlock() const;

        tempo_utils::Result<BasicBlock> addBasicBlock();

        void print();

    private:
        std::shared_ptr<internal::GraphPriv> m_priv;

        friend class print_visitor;
    };
}

#endif // LYRIC_OPTIMIZER_CONTROL_FLOW_GRAPH_H
