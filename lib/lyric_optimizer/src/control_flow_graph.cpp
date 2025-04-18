
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

#include <lyric_assembler/assembler_instructions.h>
#include <lyric_optimizer/control_flow_graph.h>
#include <lyric_optimizer/internal/cfg_data.h>
#include <lyric_optimizer/optimizer_result.h>
#include <lyric_optimizer/value.h>
#include <lyric_optimizer/variable.h>
#include <tempo_utils/status.h>

lyric_optimizer::ControlFlowGraph::ControlFlowGraph()
{
}

inline char chr(tu_uint8 value) {
    if (value >= 26)
        return '?';
    char c = static_cast<char>(0x61) + value;
    return c;
}

inline std::string counter_to_name(int counter)
{
    std::string name;
    while (counter >= 26) {
        tu_uint8 next = counter / 26;
        name.push_back(chr(next));
        counter = counter - (26 * next);
    }
    name.push_back(chr(counter));
    return name;
}

inline std::shared_ptr<lyric_optimizer::internal::VariablePriv>
create_variable(
    lyric_assembler::SymbolType type,
    int offset,
    int variableCount)
{
    auto variable = std::make_shared<lyric_optimizer::internal::VariablePriv>();
    variable->name = counter_to_name(variableCount);
    variable->type = type;
    variable->offset = offset;
    variable->counter = 0;
    return variable;
}

lyric_optimizer::ControlFlowGraph::ControlFlowGraph(
    tu_int32 numArguments,
    tu_int32 numLocals,
    tu_int32 numLexicals)
{
    m_priv = std::make_shared<internal::GraphPriv>();
    m_priv->counter = 0;

    int variableCount = 0;

    // allocate variables
    for (int i = 0; i < numArguments; i++) {
        auto variable = create_variable(
            lyric_assembler::SymbolType::ARGUMENT, i, variableCount++);
        m_priv->arguments.push_back(variable);
        m_priv->variables[variable->name] = variable;
    }
    for (int i = 0; i < numLocals; i++) {
        auto variable = create_variable(
            lyric_assembler::SymbolType::LOCAL, i, variableCount++);
        m_priv->locals.push_back(variable);
        m_priv->variables[variable->name] = variable;
    }
    for (int i = 0; i < numLexicals; i++) {
        auto variable = create_variable(
            lyric_assembler::SymbolType::LEXICAL, i, variableCount++);
        m_priv->lexicals.push_back(variable);
        m_priv->variables[variable->name] = variable;
    }

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
    if (m_priv->counter == internal::kCounterMax - 1)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "exceeded maximum basic blocks");

    // create the vertex and basic block
    auto vertex = boost::add_vertex(m_priv->graph);
    auto basicBlock = std::make_shared<internal::BasicBlockPriv>();
    basicBlock->id = m_priv->counter++;

    // link the block to the vertex and vice versa
    basicBlock->blockVertex = vertex;
    m_priv->vertexBlocks[vertex] = basicBlock;

    return BasicBlock(basicBlock, m_priv);
}

lyric_optimizer::Variable
lyric_optimizer::ControlFlowGraph::getArgument(int offset) const
{
    if (m_priv == nullptr)
        return {};
    if (m_priv->arguments.size() <= offset)
        return {};
    auto variable = m_priv->arguments.at(offset);
    return Variable(variable, m_priv);
}

lyric_optimizer::Variable
lyric_optimizer::ControlFlowGraph::resolveArgument(lyric_assembler::ArgumentVariable *argumentVariable) const
{
    if (m_priv == nullptr)
        return {};
    auto offset = argumentVariable->getOffset().getOffset();
    if (m_priv->arguments.size() <= offset)
        return {};
    auto variable = m_priv->arguments.at(offset);
    return Variable(variable, m_priv);
}

int
lyric_optimizer::ControlFlowGraph::numArguments() const
{
    if (m_priv == nullptr)
        return {};
    return m_priv->arguments.size();
}

lyric_optimizer::Variable
lyric_optimizer::ControlFlowGraph::getLocal(int offset) const
{
    if (m_priv == nullptr)
        return {};
    if (m_priv->locals.size() <= offset)
        return {};
    auto variable = m_priv->locals.at(offset);
    return Variable(variable, m_priv);
}

lyric_optimizer::Variable
lyric_optimizer::ControlFlowGraph::resolveLocal(lyric_assembler::LocalVariable *localVariable) const
{
    if (m_priv == nullptr)
        return {};
    auto offset = localVariable->getOffset().getOffset();
    if (m_priv->locals.size() <= offset)
        return {};
    auto variable = m_priv->locals.at(offset);
    return Variable(variable, m_priv);
}

int
lyric_optimizer::ControlFlowGraph::numLocals() const
{
    if (m_priv == nullptr)
        return {};
    return m_priv->locals.size();
}
lyric_optimizer::Variable
lyric_optimizer::ControlFlowGraph::getLexical(int offset) const
{
    if (m_priv == nullptr)
        return {};
    if (m_priv->lexicals.size() <= offset)
        return {};
    auto variable = m_priv->lexicals.at(offset);
    return Variable(variable, m_priv);
}

lyric_optimizer::Variable
lyric_optimizer::ControlFlowGraph::resolveLexical(lyric_assembler::LexicalVariable *lexicalVariable) const
{
    if (m_priv == nullptr)
        return {};
    auto offset = lexicalVariable->getOffset().getOffset();
    if (m_priv->lexicals.size() <= offset)
        return {};
    auto variable = m_priv->lexicals.at(offset);
    return Variable(variable, m_priv);
}

int
lyric_optimizer::ControlFlowGraph::numLexicals() const
{
    if (m_priv == nullptr)
        return {};
    return m_priv->lexicals.size();
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

        if (!basicBlock->phis.empty()) {
            TU_LOG_INFO << "    phis:";
            for (const auto &phi : basicBlock->phis) {
                TU_LOG_INFO << "        " << phi.toString();
            }
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

        if (basicBlock->transfer) {
            auto &transfer = basicBlock->transfer;
            TU_LOG_INFO << "    transfer: " << lyric_object::opcode_to_name(transfer->trailer);
            if (transfer->operand) {
                TU_LOG_INFO << "        operand: " << transfer->operand->toString();
            }
        }

        if (basicBlock->next) {
            auto &next = basicBlock->next;
            if (m_priv->exit->blockVertex == boost::target(next->nextEdge, g)) {
                if (next->operand) {
                    TU_LOG_INFO << "    return: " << next->operand->toString();
                } else {
                    TU_LOG_INFO << "    return";
                }
            }
        }

        lyric_optimizer::internal::OutEdgeIterator it, end;

        TU_LOG_INFO << "    out edges:";
        for (boost::tie(it, end) = boost::out_edges(v, g); it != end; ++it) {
            auto e = *it;
            Vertex src = boost::source(e, g);
            Vertex tgt = boost::target(e, g);
            tu_uint32 targetId = target_id[e];
            const auto &labelName = label_name[e];
            if (!labelName.empty()) {
                TU_LOG_INFO << "        " << (int) m_props[src] << " -> " << (int) m_props[tgt]
                            << " label=" << labelName << " targetId=" << targetId;
            } else {
                TU_LOG_INFO << "        " << (int) m_props[src] << " -> " << (int) m_props[tgt];
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
