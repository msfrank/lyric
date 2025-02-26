#ifndef LYRIC_OPTIMIZER_OPERAND_STACK_H
#define LYRIC_OPTIMIZER_OPERAND_STACK_H

#include <stack>

#include "optimizer_directives.h"

namespace lyric_optimizer {

    class OperandStack {
    public:
        OperandStack();

        bool isEmpty() const;
        int numOperands() const;
        std::shared_ptr<AbstractDirective> peekOperand() const;
        void pushOperand(std::shared_ptr<AbstractDirective> expression);
        std::shared_ptr<AbstractDirective> popOperand();

    private:
        std::stack<std::shared_ptr<AbstractDirective>> m_stack;
    };
}

#endif // LYRIC_OPTIMIZER_OPERAND_STACK_H
