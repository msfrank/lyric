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

        bool hasLabel() const;
        std::string getLabel() const;
        tempo_utils::Status setLabel(const std::string &labelName);
        tempo_utils::Status removeLabel();

        BranchType getBranchType() const;
        lyric_object::Opcode getTrailer() const;

        tempo_utils::Result<PhiFunction> putPhiFunction(
            const std::string &name,
            const std::vector<Instance> &arguments);
        int numPhiFunctions() const;

        tempo_utils::Status appendDirective(std::shared_ptr<AbstractDirective> instruction);
        tempo_utils::Status insertDirective(
            int index,
            std::shared_ptr<AbstractDirective> instruction);
        std::shared_ptr<AbstractDirective> getDirective(int index) const;
        int numDirectives() const;

        tempo_utils::Result<Variable> getOrDeclareVariable(const std::string &name = {});
        tempo_utils::Result<Variable> getOrDeclareVariable(const lyric_assembler::LocalVariable *localVariable);
        tempo_utils::Result<Variable> resolveVariable(const std::string &name);
        tempo_utils::Result<Variable> resolveVariable(const lyric_assembler::LocalVariable *localVariable);

        bool hasPrevEdge() const;
        bool hasBranchEdge() const;
        bool hasJumpEdge() const;
        bool hasReturnEdge() const;
        bool hasNextEdge() const;

        BasicBlock getPrevBlock() const;
        tempo_utils::Status setPrevBlock(const BasicBlock &prevBlock);
        tempo_utils::Status removePrevBlock();

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
