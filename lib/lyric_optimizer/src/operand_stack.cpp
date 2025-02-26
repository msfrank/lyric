
#include <lyric_optimizer/operand_stack.h>

lyric_optimizer::OperandStack::OperandStack()
{
}

bool
lyric_optimizer::OperandStack::isEmpty() const
{
    return m_stack.empty();
}

int
lyric_optimizer::OperandStack::numOperands() const
{
    return m_stack.size();
}

std::shared_ptr<lyric_optimizer::AbstractDirective>
lyric_optimizer::OperandStack::peekOperand() const
{
    if (m_stack.empty())
        return {};
    return m_stack.top();
}

void
lyric_optimizer::OperandStack::pushOperand(std::shared_ptr<AbstractDirective> expression)
{
    m_stack.push(std::move(expression));
}

std::shared_ptr<lyric_optimizer::AbstractDirective>
lyric_optimizer::OperandStack::popOperand()
{
    if (m_stack.empty())
        return {};
    auto top = m_stack.top();
    m_stack.pop();
    return top;
}
