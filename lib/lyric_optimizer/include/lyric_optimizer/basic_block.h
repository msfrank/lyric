#ifndef LYRIC_OPTIMIZER_BASIC_BLOCK_H
#define LYRIC_OPTIMIZER_BASIC_BLOCK_H

#include <memory>
#include <lyric_assembler/assembler_instructions.h>

namespace lyric_optimizer {

    namespace internal {
        struct BasicBlockPriv;
        struct GraphPriv;
    }

    enum class BranchType {
        Invalid,
        IfNil,
        IfNotNil,
        IfTrue,
        IfFalse,
        IfZero,
        IfNotZero,
        IfGreaterThan,
        IfGreaterOrEqual,
        IfLessThan,
        IfLessOrEqual,
    };

    class BasicBlock {
    public:
        BasicBlock();
        BasicBlock(const BasicBlock &other);

        bool isValid() const;

        bool hasLabel() const;
        std::string getLabel() const;
        tempo_utils::Status setLabel(const std::string &labelName);
        tempo_utils::Status removeLabel();

        tempo_utils::Status appendInstruction(std::shared_ptr<lyric_assembler::BasicInstruction> instruction);
        tempo_utils::Status insertInstruction(
            int index,
            std::shared_ptr<lyric_assembler::BasicInstruction> instruction);
        std::shared_ptr<lyric_assembler::BasicInstruction> getInstruction(int index) const;
        int numInstructions() const;

        bool hasBranchEdge() const;
        bool hasJumpEdge() const;
        bool hasReturnEdge() const;
        bool hasNextEdge() const;

        BasicBlock getBranchBlock() const;
        tempo_utils::Status setBranchBlock(const BasicBlock &branchBlock, BranchType branch, const std::string &label);
        tempo_utils::Status removeBranchBlock();

        BasicBlock getJumpBlock() const;
        tempo_utils::Status setJumpBlock(const BasicBlock &jumpBlock, const std::string &label);
        tempo_utils::Status removeJumpBlock();

        BasicBlock getReturnBlock() const;
        tempo_utils::Status setReturnBlock();
        tempo_utils::Status removeReturnBlock();

        BasicBlock getNextBlock() const;
        tempo_utils::Status setNextBlock(const BasicBlock &nextBlock);
        tempo_utils::Status removeNextBlock();

    private:
        std::shared_ptr<internal::BasicBlockPriv> m_block;
        std::shared_ptr<internal::GraphPriv> m_graph;

        BasicBlock(
            std::shared_ptr<internal::BasicBlockPriv> block,
            std::shared_ptr<internal::GraphPriv> graph);

        friend class ControlFlowGraph;
    };
}

#endif // LYRIC_OPTIMIZER_BASIC_BLOCK_H
