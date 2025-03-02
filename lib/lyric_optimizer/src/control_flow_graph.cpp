
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

#include <lyric_assembler/assembler_instructions.h>
#include <lyric_optimizer/control_flow_graph.h>
#include <lyric_optimizer/internal/cfg_data.h>
#include <lyric_optimizer/optimizer_result.h>
#include <tempo_utils/status.h>

lyric_optimizer::ControlFlowGraph::ControlFlowGraph()
{
}

lyric_optimizer::ControlFlowGraph::ControlFlowGraph(
    tu_int32 numArguments,
    tu_int32 numLocals,
    tu_int32 numLexicals)
{
    m_priv = std::make_shared<internal::GraphPriv>();

    // allocate space for variable vectors
    m_priv->arguments.resize(numArguments);
    m_priv->locals.resize(numLocals);
    m_priv->lexicals.resize(numLexicals);

    // create the entry block
    auto entryVertex = boost::add_vertex(m_priv->graph);
    auto entryBlock = std::make_shared<internal::BasicBlockPriv>();
    entryBlock->blockVertex = entryVertex;
    m_priv->vertexBlocks[entryVertex] = entryBlock;
    m_priv->entry = entryBlock;

    // create the exit block
    auto exitVertex = boost::add_vertex(m_priv->graph);
    auto exitBlock = std::make_shared<internal::BasicBlockPriv>();
    exitBlock->blockVertex = exitVertex;
    m_priv->vertexBlocks[exitVertex] = exitBlock;
    m_priv->exit = exitBlock;
}

lyric_optimizer::ControlFlowGraph::ControlFlowGraph(const ControlFlowGraph &other)
    : m_priv(other.m_priv)
{
}

bool
lyric_optimizer::ControlFlowGraph::isValid() const
{
    return m_priv != nullptr;
}

lyric_optimizer::BasicBlock
lyric_optimizer::ControlFlowGraph::getEntryBlock() const
{
    if (m_priv == nullptr)
        return {};
    return BasicBlock(m_priv->entry, m_priv);
}

lyric_optimizer::BasicBlock
lyric_optimizer::ControlFlowGraph::getExitBlock() const
{
    if (m_priv == nullptr)
        return {};
    return BasicBlock(m_priv->exit, m_priv);
}

tempo_utils::Result<lyric_optimizer::BasicBlock>
lyric_optimizer::ControlFlowGraph::addBasicBlock()
{
    if (m_priv == nullptr)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid control flow graph");

    // create the vertex and basic block
    auto vertex = boost::add_vertex(m_priv->graph);
    auto basicBlock = std::make_shared<internal::BasicBlockPriv>();

    // link the block to the vertex and vice versa
    basicBlock->blockVertex = vertex;
    m_priv->vertexBlocks[vertex] = basicBlock;

    return BasicBlock(basicBlock, m_priv);
}

class cfg_print_visitor : public boost::default_bfs_visitor {

    typedef boost::associative_property_map<
        std::map<
            lyric_optimizer::internal::Vertex,std::size_t
    >> VertexIndexPropertyMap;

public:
    cfg_print_visitor(lyric_optimizer::internal::GraphPriv *priv)
        : m_priv(priv)
    {
        TU_ASSERT (m_priv != nullptr);
        boost::graph_traits<lyric_optimizer::internal::Graph>::vertex_iterator it, end;
        for (boost::tie(it, end) = boost::vertices(m_priv->graph); it != end; ++it) {
            lyric_optimizer::internal::Vertex v = *it;
            vertexIndexMap[v] = vertexIndexMap.size();
        }
        m_props = boost::make_assoc_property_map(vertexIndexMap);
    }

    VertexIndexPropertyMap getVertexIndex() const
    {
        return m_props;
    }

    template<typename Vertex, typename Graph>
    void discover_vertex(Vertex v, const Graph& g) const
    {
        //auto vertex_index = get(boost::vertex_index, m_props);
        auto target_id = get(lyric_optimizer::internal::target_id_t(), g);
        auto label_name = get(lyric_optimizer::internal::label_name_t(), g);

        //int index = vertex_index[v];
        int index = m_props[v];

        if (v == m_priv->entry->blockVertex) {
            TU_LOG_INFO << "vertex " << index << " (entry)";
        } else if (v == m_priv->exit->blockVertex) {
            TU_LOG_INFO << "vertex " << index << " (exit)";
        } else {
            TU_LOG_INFO << "vertex " << index;
        }
        const auto &basicBlock = m_priv->vertexBlocks.at(v);

        if (!basicBlock->name.empty()) {
            TU_LOG_INFO << "    label: " << basicBlock->name;
        }

        TU_LOG_INFO << "    block:";
        const auto &directives = basicBlock->directives;
        for (const auto &directive : directives) {
            if (directive->isValid()) {
                TU_LOG_INFO << "        " << directive->resolveDirective()->toString();
            } else {
                TU_LOG_INFO << "        ???";
            }
        }

        typename boost::graph_traits<Graph>::out_edge_iterator out_i, out_end;
        typename boost::graph_traits<Graph>::edge_descriptor e;

        TU_LOG_INFO << "    out edges:";
        for (boost::tie(out_i, out_end) = out_edges(v, g); out_i != out_end; ++out_i) {
            e = *out_i;
            Vertex src = source(e, g), targ = target(e, g);
            tu_uint32 targetId = target_id[e];
            const auto &labelName = label_name[e];
            if (!labelName.empty()) {
                TU_LOG_INFO << "        " << (int) m_props[src] << " -> " << (int) m_props[targ]
                            << " label=" << labelName << " targetId=" << targetId;
            } else {
                TU_LOG_INFO << "        " << (int) m_props[src] << " -> " << (int) m_props[targ];
            }
        }
    }

private:
    lyric_optimizer::internal::GraphPriv *m_priv;
    std::map<lyric_optimizer::internal::Vertex,std::size_t> vertexIndexMap;
    VertexIndexPropertyMap m_props;
};

void
lyric_optimizer::ControlFlowGraph::print()
{
    if (m_priv == nullptr)
        return;

    const cfg_print_visitor vis(m_priv.get());
    boost::breadth_first_search(m_priv->graph, m_priv->entry->blockVertex,
        visitor(vis).vertex_index_map(vis.getVertexIndex()));
}

std::shared_ptr<lyric_optimizer::internal::GraphPriv>
lyric_optimizer::ControlFlowGraph::getPriv() const
{
    return m_priv;
}
