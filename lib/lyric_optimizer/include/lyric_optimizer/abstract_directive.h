#ifndef LYRIC_OPTIMIZER_ABSTRACT_DIRECTIVE_H
#define LYRIC_OPTIMIZER_ABSTRACT_DIRECTIVE_H

#include <lyric_assembler/code_fragment.h>

#include "optimizer_result.h"

namespace lyric_optimizer {

    // forward declarations
    class ActivationState;
    class OperandStack;

    enum class DirectiveType {
        Invalid,
        Noop,
        Nil,
        Undef,
        Bool,
        Int,
        Float,
        Char,
        IntAdd,
        IntSub,
        IntMul,
        IntDiv,
        IntNeg,
        UseValue,
        DefineValue,
        PhiFunction,
    };

    class AbstractDirective {
    public:
        virtual ~AbstractDirective() = default;

        virtual DirectiveType getType() const = 0;

        virtual bool isExpression() const = 0;

        virtual bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const = 0;

        virtual tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) = 0;

        virtual tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) = 0;

        virtual std::string toString() const = 0;
    };
}

#endif // LYRIC_OPTIMIZER_ABSTRACT_DIRECTIVE_H
