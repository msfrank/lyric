#ifndef LYRIC_OPTIMIZER_CONTROL_FLOW_GRAPH_H
#define LYRIC_OPTIMIZER_CONTROL_FLOW_GRAPH_H

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
        int numBasicBlocks() const;

        void print();

        std::shared_ptr<internal::GraphPriv> getPriv() const;

    private:
        std::shared_ptr<internal::GraphPriv> m_priv;

        friend class print_visitor;
    };
}

#endif // LYRIC_OPTIMIZER_CONTROL_FLOW_GRAPH_H
