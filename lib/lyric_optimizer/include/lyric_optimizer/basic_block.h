#ifndef LYRIC_OPTIMIZER_BASIC_BLOCK_H
#define LYRIC_OPTIMIZER_BASIC_BLOCK_H

#include <memory>

#include <lyric_assembler/assembler_instructions.h>
#include <lyric_assembler/local_variable.h>

#include "abstract_directive.h"
#include "phi_function.h"
#include "variable.h"

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

        tu_uint32 getId() const;

        bool hasLabel() const;
        std::string getLabel() const;
        tempo_utils::Status setLabel(const std::string &labelName);
        tempo_utils::Status removeLabel();

        BranchType getBranchType() const;
        lyric_object::Opcode getTrailer() const;

        tempo_utils::Status putPhiFunction(
             const Instance &target,
             std::shared_ptr<PhiFunction> phiFunction);
        int numPhiFunctions() const;

        tempo_utils::Status appendDirective(std::shared_ptr<AbstractDirective> instruction);
        tempo_utils::Status insertDirective(
            int index,
            std::shared_ptr<AbstractDirective> instruction);
        std::shared_ptr<AbstractDirective> getDirective(int index) const;
        int numDirectives() const;

        bool hasPrevEdge() const;
        bool hasBranchEdge() const;
        bool hasJumpEdge() const;
        bool hasReturnEdge() const;
        bool hasNextEdge() const;

        BasicBlock getPrevBlock() const;
        std::vector<BasicBlock> listPredecessorBlocks() const;

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

        bool operator==(const BasicBlock &other) const;
        bool operator!=(const BasicBlock &other) const;

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
