#ifndef LYRIC_OPTIMIZER_BASIC_BLOCK_H
#define LYRIC_OPTIMIZER_BASIC_BLOCK_H

#include <memory>

#include <lyric_assembler/assembler_instructions.h>
#include <lyric_assembler/local_variable.h>

#include "abstract_directive.h"
#include "phi_function.h"
#include "value.h"
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

        lyric_object::Opcode getTrailer() const;

        tempo_utils::Status putPhiFunction(const Instance &instance);
        tempo_utils::Status removePhiFunction(const Instance &instance);
        absl::flat_hash_set<Instance>::const_iterator phiFunctionsBegin() const;
        absl::flat_hash_set<Instance>::const_iterator phiFunctionsEnd() const;
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
        std::vector<BasicBlock> listSuccessorBlocks() const;

        BasicBlock getBranchBlock() const;
        tempo_utils::Status setBranchBlock(
            const BasicBlock &branchBlock,
            BranchType branch,
            std::shared_ptr<AbstractDirective> operand);
        tempo_utils::Status removeBranchBlock();
        BranchType getBranchType() const;
        std::shared_ptr<AbstractDirective> getBranchOperand() const;

        BasicBlock getJumpBlock() const;
        tempo_utils::Status setJumpBlock(const BasicBlock &jumpBlock);
        tempo_utils::Status removeJumpBlock();

        BasicBlock getReturnBlock() const;
        tempo_utils::Status setReturnBlock(std::shared_ptr<AbstractDirective> operand);
        tempo_utils::Status removeReturnBlock();
        std::shared_ptr<AbstractDirective> getReturnOperand() const;

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
