#ifndef LYRIC_OPTIMIZER_DATA_FLOW_ANALYSIS_H
#define LYRIC_OPTIMIZER_DATA_FLOW_ANALYSIS_H

#include <vector>

#include "abstract_analysis.h"
#include "basic_block.h"
#include "control_flow_graph.h"

namespace lyric_optimizer {

    namespace internal {
        struct BasicBlockPriv;
    }

    template <class SolutionType>
    class DataFlowAnalysis {
    public:
        explicit DataFlowAnalysis(std::shared_ptr<AbstractAnalysis<SolutionType>> analysis);

        tempo_utils::Status initialize(const ControlFlowGraph &cfg);
        tempo_utils::Status run();

    private:
        std::shared_ptr<AbstractAnalysis<SolutionType>> m_analysis;

        ControlFlowGraph m_cfg;
        struct DFAState {
            bool needsEvaluation = true;
            std::shared_ptr<internal::BasicBlockPriv> blockPriv;
            SolutionType solution;
        };
        std::vector<DFAState> m_state;
        absl::flat_hash_map<std::shared_ptr<internal::BasicBlockPriv>,int> m_blockIndex;
    };
}

#endif // LYRIC_OPTIMIZER_DATA_FLOW_ANALYSIS_H
