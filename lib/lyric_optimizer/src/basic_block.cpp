
#include <lyric_optimizer/basic_block.h>
#include <lyric_optimizer/optimizer_result.h>
#include <lyric_optimizer/internal/cfg_data.h>

static tempo_utils::Result<lyric_object::Opcode> branch_type_to_opcode(lyric_optimizer::BranchType branch)
{
    switch (branch) {
        case lyric_optimizer::BranchType::IfNil:
            return lyric_object::Opcode::OP_IF_NIL;
        case lyric_optimizer::BranchType::IfNotNil:
            return lyric_object::Opcode::OP_IF_NOTNIL;
        case lyric_optimizer::BranchType::IfTrue:
            return lyric_object::Opcode::OP_IF_TRUE;
        case lyric_optimizer::BranchType::IfFalse:
            return lyric_object::Opcode::OP_IF_FALSE;
        case lyric_optimizer::BranchType::IfZero:
            return lyric_object::Opcode::OP_IF_ZERO;
        case lyric_optimizer::BranchType::IfNotZero:
            return lyric_object::Opcode::OP_IF_NOTZERO;
        case lyric_optimizer::BranchType::IfGreaterThan:
            return lyric_object::Opcode::OP_IF_GT;
        case lyric_optimizer::BranchType::IfGreaterOrEqual:
            return lyric_object::Opcode::OP_IF_GE;
        case lyric_optimizer::BranchType::IfLessThan:
            return lyric_object::Opcode::OP_IF_LT;
        case lyric_optimizer::BranchType::IfLessOrEqual:
            return lyric_object::Opcode::OP_IF_LE;
        default:
            return lyric_optimizer::OptimizerStatus::forCondition(
                lyric_optimizer::OptimizerCondition::kOptimizerInvariant, "invalid branch type");
    }
}

lyric_optimizer::BasicBlock::BasicBlock()
{
}

lyric_optimizer::BasicBlock::BasicBlock(const BasicBlock &other)
    : m_block(other.m_block),
      m_graph(other.m_graph)
{
}

lyric_optimizer::BasicBlock::BasicBlock(
    std::shared_ptr<internal::BasicBlockPriv> block,
    std::shared_ptr<internal::GraphPriv> graph)
    : m_block(std::move(block)),
      m_graph(std::move(graph))
{
    TU_ASSERT (m_block != nullptr);
    TU_ASSERT (m_graph != nullptr);
}

bool
lyric_optimizer::BasicBlock::isValid() const
{
    return m_block != nullptr;
}

bool
lyric_optimizer::BasicBlock::hasLabel() const
{
    if (m_block == nullptr)
        return false;
    return !m_block->name.empty();
}

std::string
lyric_optimizer::BasicBlock::getLabel() const
{
    if (m_block == nullptr)
        return {};
    return m_block->name;
}

tempo_utils::Status
lyric_optimizer::BasicBlock::setLabel(const std::string &labelName)
{
    if (m_block == nullptr)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid basic block");
    TU_RETURN_IF_NOT_OK (removeLabel());
    m_block->name = labelName;
    return {};
}

tempo_utils::Status
lyric_optimizer::BasicBlock::removeLabel()
{
    if (m_block == nullptr)
        return {};
    if (m_block->name.empty())
        return {};

    // remove all labeled in-edges
    internal::InEdgeIterator it, end;
    std::tie(it, end) = boost::in_edges(m_block->blockVertex, m_graph->graph);
    auto label_name = get(internal::label_name_t(), m_graph->graph);
    for (; it != end; ++it) {
        auto e = *it;
        auto target = boost::source(e, m_graph->graph);
        const auto &labelName = label_name[e];
        if (!labelName.empty()) {
            boost::remove_edge(e, m_graph->graph);
            auto &block = m_graph->vertexBlocks[target];
            block->transfer = {};
        }
    }
    return {};
}

lyric_optimizer::BranchType
lyric_optimizer::BasicBlock::getBranchType() const
{
    if (m_block == nullptr)
        return {};
    if (m_block->transfer == nullptr)
        return BranchType::Invalid;
    switch (m_block->transfer->trailer) {
        case lyric_object::Opcode::OP_IF_NIL:
            return BranchType::IfNil;
        case lyric_object::Opcode::OP_IF_NOTNIL:
            return BranchType::IfNotNil;
        case lyric_object::Opcode::OP_IF_TRUE:
            return BranchType::IfTrue;
        case lyric_object::Opcode::OP_IF_FALSE:
            return BranchType::IfFalse;
        case lyric_object::Opcode::OP_IF_ZERO:
            return BranchType::IfZero;
        case lyric_object::Opcode::OP_IF_NOTZERO:
            return BranchType::IfNotZero;
        case lyric_object::Opcode::OP_IF_GT:
            return BranchType::IfGreaterThan;
        case lyric_object::Opcode::OP_IF_GE:
            return BranchType::IfGreaterOrEqual;
        case lyric_object::Opcode::OP_IF_LT:
            return BranchType::IfLessThan;
        case lyric_object::Opcode::OP_IF_LE:
            return BranchType::IfLessOrEqual;
        default:
            return BranchType::Invalid;
    }
}

lyric_object::Opcode
lyric_optimizer::BasicBlock::getTrailer() const
{
    if (m_block == nullptr)
        return {};
    if (m_block->transfer == nullptr)
        return lyric_object::Opcode::OP_UNKNOWN;
    return m_block->transfer->trailer;
}

tempo_utils::Result<lyric_optimizer::PhiFunction>
lyric_optimizer::BasicBlock::putPhiFunction(const std::string &name, const std::vector<Instance> &arguments)
{
    if (m_block == nullptr)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid basic block");
    auto entry = m_graph->variables.find(name);
    if (entry == m_graph->variables.cend())
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing variable {}", name);
    Variable variable(entry->second, m_graph);

    auto phi = std::make_shared<internal::PhiPriv>();
    TU_ASSIGN_OR_RETURN (phi->target, variable.pushInstance());
    phi->arguments = arguments;
    m_block->phis[name] = phi;

    return PhiFunction(phi);
}

int
lyric_optimizer::BasicBlock::numPhiFunctions() const
{
    return m_block->phis.size();
}

tempo_utils::Status
lyric_optimizer::BasicBlock::appendDirective(std::shared_ptr<AbstractDirective> directive)
{
    TU_ASSERT (directive != nullptr);
    if (m_block == nullptr)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid basic block");
    auto chain = std::make_shared<DirectiveChain>(directive);
    m_block->directives.push_back(chain);
    return {};
}

tempo_utils::Status
lyric_optimizer::BasicBlock::insertDirective(
    int index,
    std::shared_ptr<AbstractDirective> directive)
{
    if (m_block == nullptr)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid basic block");

    auto chain = std::make_shared<DirectiveChain>(directive);
    if (index <= 0) {
        m_block->directives.insert(m_block->directives.begin(), chain);
    } else {
        auto size = m_block->directives.size();
        if (size < index) {
            index = size;
        }
        auto it = m_block->directives.begin();
        std::advance(it, index);
        m_block->directives.insert(it, chain);
    }
    return {};
}

std::shared_ptr<lyric_optimizer::AbstractDirective>
lyric_optimizer::BasicBlock::getDirective(int index) const
{
    if (m_block == nullptr)
        return {};
    if (m_block->directives.size() <= index)
        return {};
    auto chain = m_block->directives.at(index);
    return chain->resolveDirective();
}

int
lyric_optimizer::BasicBlock::numDirectives() const
{
    if (m_block == nullptr)
        return 0;
    return m_block->directives.size();
}

tempo_utils::Result<lyric_optimizer::Variable>
lyric_optimizer::BasicBlock::getOrDeclareVariable(const std::string &name)
{
    auto entry = m_graph->variables.find(name);
    if (entry != m_graph->variables.cend())
        return Variable(entry->second, m_graph);

    // create the variable
    auto variable = std::make_shared<internal::VariablePriv>();
    variable->name = name;
    variable->declaration = m_block;
    variable->counter = 0;

    // link the variable to the cfg
    m_graph->variables[name] = variable;

    // link the declaration to the block
    m_block->declarations[name] = variable;

    return Variable(variable, m_graph);
}

tempo_utils::Result<lyric_optimizer::Variable>
lyric_optimizer::BasicBlock::getOrDeclareVariable(const lyric_assembler::LocalVariable *localVariable)
{
    return getOrDeclareVariable(localVariable->getSymbolUrl().toString());
}

tempo_utils::Result<lyric_optimizer::Variable>
lyric_optimizer::BasicBlock::resolveVariable(const std::string &name)
{
    auto entry = m_graph->variables.find(name);
    if (entry == m_graph->variables.cend())
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing variable {}", name);
    Variable variable(entry->second, m_graph);
    return variable;
}

tempo_utils::Result<lyric_optimizer::Variable>
lyric_optimizer::BasicBlock::resolveVariable(const lyric_assembler::LocalVariable *localVariable)
{
    return resolveVariable(localVariable->getSymbolUrl().toString());
}

bool
lyric_optimizer::BasicBlock::hasPrevEdge() const
{
    if (m_block == nullptr)
        return false;
    return m_block->prev != nullptr;
}

bool
lyric_optimizer::BasicBlock::hasJumpEdge() const
{
    if (m_block == nullptr)
        return false;
    if (m_block->transfer == nullptr)
        return false;
    return m_block->transfer->trailer == lyric_object::Opcode::OP_JUMP;
}

bool
lyric_optimizer::BasicBlock::hasBranchEdge() const
{
    if (m_block == nullptr)
        return false;
    if (m_block->transfer == nullptr)
        return false;
    return m_block->transfer->trailer != lyric_object::Opcode::OP_JUMP;
}

bool
lyric_optimizer::BasicBlock::hasReturnEdge() const
{
    if (m_block == nullptr)
        return false;
    if (m_block->next == nullptr)
        return false;
    auto target = boost::target(m_block->next->nextEdge, m_graph->graph);
    return target == m_graph->exit->blockVertex;
}

bool
lyric_optimizer::BasicBlock::hasNextEdge() const
{
    if (m_block == nullptr)
        return false;
    if (m_block->next == nullptr)
        return false;
    auto target = boost::target(m_block->next->nextEdge, m_graph->graph);
    return target != m_graph->exit->blockVertex;
}

lyric_optimizer::BasicBlock
lyric_optimizer::BasicBlock::getPrevBlock() const
{
    if (m_block == nullptr)
        return {};
    if (m_block->prev == nullptr)
        return {};
    auto source = boost::source(m_block->prev->prevEdge, m_graph->graph);
    auto block = m_graph->vertexBlocks.at(source);
    return BasicBlock(block, m_graph);
}

// tempo_utils::Status
// lyric_optimizer::BasicBlock::setNextBlock(const BasicBlock &nextBlock)
// {
//     if (m_block == nullptr)
//         return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
//             "invalid basic block");
//     if (!nextBlock.isValid())
//         return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
//             "invalid next block");
//     if (nextBlock.hasPrevEdge())
//         return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
//             "next block is already a successor");
//     TU_RETURN_IF_NOT_OK (removeReturnBlock());
//     TU_RETURN_IF_NOT_OK (removeNextBlock());
//     auto ret = boost::add_edge(m_block->blockVertex, nextBlock.m_block->blockVertex, m_graph->graph);
//     if (ret.second == false)
//         return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
//             "next edge already exists");
//     m_block->next = std::make_unique<internal::NextPriv>();
//     m_block->next->nextEdge = ret.first;
//     nextBlock.m_block->prev = std::make_unique<internal::PrevPriv>();
//     nextBlock.m_block->prev->prevEdge = ret.first;
//     return {};
// }
//
// tempo_utils::Status
// lyric_optimizer::BasicBlock::removeNextBlock()
// {
//     if (m_block == nullptr)
//         return {};
//     if (m_block->next == nullptr)
//         return {};
//     auto target = boost::target(m_block->next->nextEdge, m_graph->graph);
//     if (target == m_graph->exit->blockVertex)
//         return {};
//     auto nextBlock = m_graph->vertexBlocks.at(target);
//     nextBlock->prev.reset();
//     boost::remove_edge(m_block->next->nextEdge, m_graph->graph);
//     m_block->next.reset();
//     return {};
// }

lyric_optimizer::BasicBlock
lyric_optimizer::BasicBlock::getBranchBlock() const
{
    if (m_block == nullptr)
        return {};
    if (m_block->transfer == nullptr)
        return {};
    if (m_block->transfer->trailer == lyric_object::Opcode::OP_JUMP)
        return {};
    auto target = boost::target(m_block->transfer->transferEdge, m_graph->graph);
    auto block = m_graph->vertexBlocks.at(target);
    return BasicBlock(block, m_graph);
}

tempo_utils::Status
lyric_optimizer::BasicBlock::setBranchBlock(
    const BasicBlock &branchBlock,
    BranchType branch,
    const std::string &label)
{
    if (m_block == nullptr)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid basic block");
    if (!branchBlock.isValid())
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid branch block");
    lyric_object::Opcode trailer;
    TU_ASSIGN_OR_RETURN (trailer, branch_type_to_opcode(branch));
    TU_RETURN_IF_NOT_OK (removeBranchBlock());
    TU_RETURN_IF_NOT_OK (removeJumpBlock());

    auto label_name = boost::get(internal::label_name_t(), m_graph->graph);
    auto ret = boost::add_edge(m_block->blockVertex, branchBlock.m_block->blockVertex, m_graph->graph);
    if (ret.second == false)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "branch edge already exists");
    boost::put(label_name, ret.first, label);
    m_block->transfer = std::make_unique<internal::TransferPriv>();
    m_block->transfer->transferEdge = ret.first;
    m_block->transfer->trailer = trailer;
    return {};
}

tempo_utils::Status
lyric_optimizer::BasicBlock::removeBranchBlock()
{
    if (m_block == nullptr)
        return {};
    if (m_block->transfer == nullptr)
        return {};
    if (m_block->transfer->trailer == lyric_object::Opcode::OP_JUMP)
        return {};
    boost::remove_edge(m_block->transfer->transferEdge, m_graph->graph);
    m_block->transfer.reset();
    return {};
}

lyric_optimizer::BasicBlock
lyric_optimizer::BasicBlock::getJumpBlock() const
{
    if (m_block == nullptr)
        return {};
    if (m_block->transfer == nullptr)
        return {};
    if (m_block->transfer->trailer != lyric_object::Opcode::OP_JUMP)
        return {};
    auto target = boost::target(m_block->transfer->transferEdge, m_graph->graph);
    auto block = m_graph->vertexBlocks.at(target);
    return BasicBlock(block, m_graph);
}

tempo_utils::Status
lyric_optimizer::BasicBlock::setJumpBlock(const BasicBlock &jumpBlock, const std::string &label)
{
    if (m_block == nullptr)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid basic block");
    if (!jumpBlock.isValid())
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid jump block");
    TU_RETURN_IF_NOT_OK (removeBranchBlock());
    TU_RETURN_IF_NOT_OK (removeJumpBlock());

    auto label_name = boost::get(internal::label_name_t(), m_graph->graph);
    auto ret = boost::add_edge(m_block->blockVertex, jumpBlock.m_block->blockVertex, m_graph->graph);
    if (ret.second == false)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "jump edge already exists");
    boost::put(label_name, ret.first, label);
    m_block->transfer = std::make_unique<internal::TransferPriv>();
    m_block->transfer->transferEdge = ret.first;
    m_block->transfer->trailer = lyric_object::Opcode::OP_JUMP;
    return {};
}

tempo_utils::Status
lyric_optimizer::BasicBlock::removeJumpBlock()
{
    if (m_block == nullptr)
        return {};
    if (m_block->transfer == nullptr)
        return {};
    if (m_block->transfer->trailer != lyric_object::Opcode::OP_JUMP)
        return {};
    boost::remove_edge(m_block->transfer->transferEdge, m_graph->graph);
    m_block->transfer.reset();
    return {};
}

lyric_optimizer::BasicBlock
lyric_optimizer::BasicBlock::getReturnBlock() const
{
    if (m_block == nullptr)
        return {};
    if (m_block->next == nullptr)
        return {};
    auto target = boost::target(m_block->next->nextEdge, m_graph->graph);
    if (target != m_graph->exit->blockVertex)
        return {};
    return BasicBlock(m_graph->exit, m_graph);
}

tempo_utils::Status
lyric_optimizer::BasicBlock::setReturnBlock()
{
    if (m_block == nullptr)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid basic block");
    TU_RETURN_IF_NOT_OK (removeReturnBlock());
    TU_RETURN_IF_NOT_OK (removeNextBlock());
    auto ret = boost::add_edge(m_block->blockVertex, m_graph->exit->blockVertex, m_graph->graph);
    if (ret.second == false)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "next edge already exists");
    m_block->next = std::make_unique<internal::NextPriv>();
    m_block->next->nextEdge = ret.first;
    return {};
}

tempo_utils::Status
lyric_optimizer::BasicBlock::removeReturnBlock()
{
    if (m_block == nullptr)
        return {};
    if (m_block->next == nullptr)
        return {};
    auto target = boost::target(m_block->next->nextEdge, m_graph->graph);
    if (target != m_graph->exit->blockVertex)
        return {};
    boost::remove_edge(m_block->next->nextEdge, m_graph->graph);
    m_block->next.reset();
    return {};
}

lyric_optimizer::BasicBlock
lyric_optimizer::BasicBlock::getNextBlock() const
{
    if (m_block == nullptr)
        return {};
    if (m_block->next == nullptr)
        return {};
    auto target = boost::target(m_block->next->nextEdge, m_graph->graph);
    if (target == m_graph->exit->blockVertex)
        return {};
    auto block = m_graph->vertexBlocks.at(target);
    return BasicBlock(block, m_graph);
}

tempo_utils::Status
lyric_optimizer::BasicBlock::setNextBlock(const BasicBlock &nextBlock)
{
    if (m_block == nullptr)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid basic block");
    if (!nextBlock.isValid())
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid next block");
    if (nextBlock.hasPrevEdge())
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "next block is already a successor");
    TU_RETURN_IF_NOT_OK (removeReturnBlock());
    TU_RETURN_IF_NOT_OK (removeNextBlock());
    auto ret = boost::add_edge(m_block->blockVertex, nextBlock.m_block->blockVertex, m_graph->graph);
    if (ret.second == false)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "next edge already exists");
    m_block->next = std::make_unique<internal::NextPriv>();
    m_block->next->nextEdge = ret.first;
    nextBlock.m_block->prev = std::make_unique<internal::PrevPriv>();
    nextBlock.m_block->prev->prevEdge = ret.first;
    return {};
}

tempo_utils::Status
lyric_optimizer::BasicBlock::removeNextBlock()
{
    if (m_block == nullptr)
        return {};
    if (m_block->next == nullptr)
        return {};
    auto target = boost::target(m_block->next->nextEdge, m_graph->graph);
    if (target == m_graph->exit->blockVertex)
        return {};
    auto nextBlock = m_graph->vertexBlocks.at(target);
    nextBlock->prev.reset();
    boost::remove_edge(m_block->next->nextEdge, m_graph->graph);
    m_block->next.reset();
    return {};
}

bool
lyric_optimizer::BasicBlock::operator==(const BasicBlock &other) const
{
    if (!m_graph && !other.m_graph)
        return false;
    if (m_graph != other.m_graph)
        return false;
    return m_block == other.m_block;
}

bool
lyric_optimizer::BasicBlock::operator!=(const BasicBlock &other) const
{
    return !(*this == other);
}
