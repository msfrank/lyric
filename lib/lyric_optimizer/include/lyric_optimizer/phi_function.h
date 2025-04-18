#ifndef LYRIC_OPTIMIZER_PHI_FUNCTION_H
#define LYRIC_OPTIMIZER_PHI_FUNCTION_H

#include <vector>

#include "abstract_directive.h"
#include "instance.h"

namespace lyric_optimizer {

    class PhiFunction : public AbstractDirective {
    public:
        explicit PhiFunction(const absl::flat_hash_set<Instance> &arguments);

        DirectiveType getType() const override;
        bool isExpression() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;

        absl::flat_hash_set<Instance>::const_iterator argumentsBegin() const;
        absl::flat_hash_set<Instance>::const_iterator argumentsEnd() const;
        int numArguments() const;

    private:
        absl::flat_hash_set<Instance> m_arguments;

        friend class BasicBlock;
    };
}

#endif // LYRIC_OPTIMIZER_PHI_FUNCTION_H
