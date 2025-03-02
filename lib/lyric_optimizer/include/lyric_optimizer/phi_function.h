#ifndef LYRIC_OPTIMIZER_PHI_FUNCTION_H
#define LYRIC_OPTIMIZER_PHI_FUNCTION_H

#include <vector>

#include "abstract_directive.h"
#include "instance.h"

namespace lyric_optimizer {

    class PhiFunction : public AbstractDirective {
    public:
        explicit PhiFunction(const std::vector<Instance> &arguments);

        DirectiveType getType() const override;
        bool isExpression() const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;

        std::vector<Instance>::const_iterator argumentsBegin() const;
        std::vector<Instance>::const_iterator argumentsEnd() const;
        int numArguments() const;

    private:
        std::vector<Instance> m_arguments;

        friend class BasicBlock;
    };
}

#endif // LYRIC_OPTIMIZER_PHI_FUNCTION_H
