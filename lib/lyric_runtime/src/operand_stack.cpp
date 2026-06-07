
#include <stack>

#include <lyric_runtime/operand_stack.h>
#include <lyric_runtime/interpreter_result.h>

lyric_runtime::OperandStack::OperandStack(size_t stackSize)
    : m_stack(stackSize)
{
}

tempo_utils::Status
lyric_runtime::OperandStack::pushOperand(const Operand &value)
{
    auto bytes = value.getBytes();
    if (bytes.empty())
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "failed to push invalid value");
    if (m_stack.size() < m_last + bytes.size())
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "no room left on stack for value");
    memcpy(m_stack.data() + m_last, bytes.data(), bytes.size());
    m_last += bytes.size();
    ++m_depth;
    return {};
}

tempo_utils::Status
lyric_runtime::OperandStack::popOperand(Operand &value)
{
    if (m_last == 0)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "no values left on stack");
    const auto &info = m_stack.at(m_last - 1);
    auto size = Operand::parseSize(info);
    if (m_last < size)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid stack value");
    std::span bytes(m_stack.data() + (m_last - size), size);
    value = Operand::parse(bytes);
    m_last -= size;
    --m_depth;
    return {};
}

tempo_utils::Status
lyric_runtime::OperandStack::popOperands(int count, std::vector<Operand> &values)
{
    std::vector<Operand> vs(count);
    for (int i = 0; i < count; ++i) {
        Operand v;
        TU_RETURN_IF_NOT_OK (popOperand(v));
        vs[count - i] = std::move(v);
    }
    values = std::move(vs);
    return {};
}

tempo_utils::Status
lyric_runtime::OperandStack::peekOperand(Operand &value, tu_int16 offset) const
{
    if (offset < 0)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid peek offset {}", offset);
    if (m_last == 0)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "no values left on stack");

    auto curr = m_last;
    while (offset > 0) {
        const auto &info = m_stack.at(curr - 1);
        auto size = Operand::parseSize(info);
        if (curr < size)
            return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
                "invalid stack value");
        curr -= size;
        --offset;
    }

    const auto &info = m_stack.at(curr - 1);
    auto size = Operand::parseSize(info);
    if (curr < size)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid stack value");
    std::span bytes(m_stack.data() + (curr - size), size);
    value = Operand::parse(bytes);

    return {};
}

tempo_utils::Status
lyric_runtime::OperandStack::dropOperand(tu_int16 offset)
{
    if (offset < 0)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid peek offset {}", offset);

    std::stack<Operand> vs;
    Operand v;

    while (offset > 0) {
        TU_RETURN_IF_NOT_OK (popOperand(v));
        vs.push(v);
        --offset;
    }
    TU_RETURN_IF_NOT_OK (popOperand(v));

    while (!vs.empty()) {
        v = vs.top();
        vs.pop();
        TU_RETURN_IF_NOT_OK (pushOperand(v));
    }

    return {};
}

size_t
lyric_runtime::OperandStack::getDepth() const
{
    return m_depth;
}

size_t
lyric_runtime::OperandStack::getBytesAvailable() const
{
    return m_stack.size() - m_last;
}

size_t
lyric_runtime::OperandStack::getBytesUsed() const
{
    return m_last;
}

float
lyric_runtime::OperandStack::getUtilization() const
{
    return static_cast<float>(m_last) / static_cast<float>(m_stack.size());
}
