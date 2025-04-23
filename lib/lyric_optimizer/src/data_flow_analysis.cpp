
#include <boost/graph/topological_sort.hpp>

#include <lyric_optimizer/data_flow_analysis.h>
#include <lyric_optimizer/internal/cfg_data.h>

template<class SolutionType>
lyric_optimizer::DataFlowAnalysis<SolutionType>::DataFlowAnalysis(
    std::shared_ptr<AbstractAnalysis<SolutionType>> analysis)
    : m_analysis(std::move(analysis))
{
    TU_ASSERT (m_analysis != nullptr);
}

template<class SolutionType>
tempo_utils::Status
lyric_optimizer::DataFlowAnalysis<SolutionType>::initialize(const ControlFlowGraph &cfg)
{
    if (!m_state.empty())
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "DataFlowFramework is already initialized");

    m_cfg = cfg;
    TU_ASSERT (m_cfg.isValid());

    auto graphPriv = m_cfg.getPriv();

    // construct a topological ordering of the basic blocks
    std::deque<internal::Vertex> topologicalOrder;
    boost::topological_sort(graphPriv->graph, std::front_inserter(topologicalOrder),
        boost::vertex_index_map(boost::identity_property_map()));

    // construct the dfa state
    for (auto &vertex : topologicalOrder) {
        int index = m_state.size();
        auto blockPriv = graphPriv->vertexBlocks.at(vertex);
        BasicBlock basicBlock(blockPriv, graphPriv);

        DFAState state;
        state.needsEvaluation = true;
        state.blockPriv = blockPriv;
        state.solution = std::move(m_analysis->initialize(state.block));
        m_state.push_back(std::move(state));
        m_blockIndex[blockPriv] = index;
    }

    return {};
}

template<class SolutionType>
tempo_utils::Status
lyric_optimizer::DataFlowAnalysis<SolutionType>::run()
{
    auto graphPriv = m_cfg.getPriv();
    auto g = graphPriv->graph;

    bool finished;
    do {
        finished = true;
        for (auto &state : m_state) {
            if (state.needsEvaluation) {
                state.needsEvaluation = false;
                auto prevSolution = state.solution;

                auto v = state.priv->blockVertex;
                internal::InEdgeIterator in_it, in_end;

                std::vector<SolutionType> predecessors;
                for (boost::tie(in_it, in_end) = boost::in_edges(v, g); in_it != in_end; ++in_it) {
                    auto s = boost::source(*in_it, g);
                    const auto &pred = m_state.at(m_blockIndex.at(s));
                    predecessors.push_back(pred.solution);
                }

                auto input = std::move(m_analysis->meet(predecessors));
                state.solution = std::move(m_analysis->transfer(input));

                if (state.solution != prevSolution) {
                    internal::OutEdgeIterator out_it, out_end;

                    for (boost::tie(out_it, out_end) = boost::in_edges(v, g); out_it != out_end; ++out_it) {
                        auto t = boost::target(*out_it, g);
                        auto &succ = m_state.at(m_blockIndex.at(t));
                        succ.needsEvaluation = true;
                    }
                    // FIXME: finished = finished || t is a back edge of v
                    finished = false;
                }
            }
        }
    } while (!finished);

    return {};
}
