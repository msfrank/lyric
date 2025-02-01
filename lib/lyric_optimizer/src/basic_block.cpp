
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
            block->label = {};
        }
    }
    return {};
}

tempo_utils::Status
lyric_optimizer::BasicBlock::appendInstruction(std::shared_ptr<lyric_assembler::BasicInstruction> instruction)
{
    if (m_block == nullptr)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid basic block");
    m_block->instructions.push_back(instruction);
    return {};
}

tempo_utils::Status
lyric_optimizer::BasicBlock::insertInstruction(
    int index,
    std::shared_ptr<lyric_assembler::BasicInstruction> instruction)
{
    if (m_block == nullptr)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid basic block");
    if (index <= 0) {
        m_block->instructions.insert(m_block->instructions.begin(), instruction);
    } else {
        auto size = m_block->instructions.size();
        if (size < index) {
            index = size;
        }
        auto it = m_block->instructions.begin();
        std::advance(it, index);
        m_block->instructions.insert(it, instruction);
    }
    return {};
}

std::shared_ptr<lyric_assembler::BasicInstruction>
lyric_optimizer::BasicBlock::getInstruction(int index) const
{
    if (m_block == nullptr)
        return {};
    if (m_block->instructions.size() <= index)
        return {};
    return m_block->instructions.at(index);
}

int
lyric_optimizer::BasicBlock::numInstructions() const
{
    if (m_block == nullptr)
        return 0;
    return m_block->instructions.size();
}

bool
lyric_optimizer::BasicBlock::hasJumpEdge() const
{
    if (m_block == nullptr)
        return false;
    if (m_block->label == nullptr)
        return false;
    return m_block->label->trailer == lyric_object::Opcode::OP_JUMP;
}

bool
lyric_optimizer::BasicBlock::hasBranchEdge() const
{
    if (m_block == nullptr)
        return false;
    if (m_block->label == nullptr)
        return false;
    return m_block->label->trailer != lyric_object::Opcode::OP_JUMP;
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
lyric_optimizer::BasicBlock::getBranchBlock() const
{
    if (m_block == nullptr)
        return {};
    if (m_block->label == nullptr)
        return {};
    if (m_block->label->trailer == lyric_object::Opcode::OP_JUMP)
        return {};
    auto target = boost::target(m_block->label->labelEdge, m_graph->graph);
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
    m_block->label = std::make_unique<internal::LabelPriv>();
    m_block->label->labelEdge = ret.first;
    m_block->label->trailer = trailer;
    return {};
}

tempo_utils::Status
lyric_optimizer::BasicBlock::removeBranchBlock()
{
    if (m_block == nullptr)
        return {};
    if (m_block->label == nullptr)
        return {};
    if (m_block->label->trailer == lyric_object::Opcode::OP_JUMP)
        return {};
    boost::remove_edge(m_block->label->labelEdge, m_graph->graph);
    m_block->label.reset();
    return {};
}

lyric_optimizer::BasicBlock
lyric_optimizer::BasicBlock::getJumpBlock() const
{
    if (m_block == nullptr)
        return {};
    if (m_block->label == nullptr)
        return {};
    if (m_block->label->trailer != lyric_object::Opcode::OP_JUMP)
        return {};
    auto target = boost::target(m_block->label->labelEdge, m_graph->graph);
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
    m_block->label = std::make_unique<internal::LabelPriv>();
    m_block->label->labelEdge = ret.first;
    m_block->label->trailer = lyric_object::Opcode::OP_JUMP;
    return {};
}

tempo_utils::Status
lyric_optimizer::BasicBlock::removeJumpBlock()
{
    if (m_block == nullptr)
        return {};
    if (m_block->label == nullptr)
        return {};
    if (m_block->label->trailer != lyric_object::Opcode::OP_JUMP)
        return {};
    boost::remove_edge(m_block->label->labelEdge, m_graph->graph);
    m_block->label.reset();
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
    TU_RETURN_IF_NOT_OK (removeReturnBlock());
    TU_RETURN_IF_NOT_OK (removeNextBlock());
    auto ret = boost::add_edge(m_block->blockVertex, nextBlock.m_block->blockVertex, m_graph->graph);
    if (ret.second == false)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "next edge already exists");
    m_block->next = std::make_unique<internal::NextPriv>();
    m_block->next->nextEdge = ret.first;
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
    boost::remove_edge(m_block->next->nextEdge, m_graph->graph);
    m_block->next.reset();
    return {};
}
