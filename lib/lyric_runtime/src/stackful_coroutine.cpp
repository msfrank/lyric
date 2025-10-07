
#include <lyric_runtime/interpreter_result.h>
#include <lyric_runtime/stackful_coroutine.h>

/**
 *
 * @tparam T stack element type
 * @param stack stack to operate on
 * @param offset the specified offset, which can be negative
 * @return the absolute index, or -1 if the offset is out of bounds
 */
template<typename T>
int
calculate_stack_index(const std::vector<T> &stack, int offset)
{
    auto size = stack.size();
    if (offset < 0) {
        offset = size + offset;
    }
    return offset < size? offset : -1;
}

lyric_runtime::StackfulCoroutine::StackfulCoroutine()
    : m_IP(),
      m_SP(nullptr)
{
}

bool
lyric_runtime::StackfulCoroutine::nextOp(lyric_object::OpCell &op)
{
    return m_IP.getNext(op);
}

bool
lyric_runtime::StackfulCoroutine::moveIP(int16_t offset)
{
    return m_IP.move(offset);
}

lyric_runtime::BytecodeSegment *
lyric_runtime::StackfulCoroutine::peekSP() const
{
    return m_SP;
}

lyric_object::BytecodeIterator
lyric_runtime::StackfulCoroutine::peekIP() const
{
    return m_IP;
}

void
lyric_runtime::StackfulCoroutine::transferControl(const lyric_object::BytecodeIterator &ip, BytecodeSegment *sp)
{
    m_IP = ip;
    if (sp != nullptr) {
        m_SP = sp;
    }
}

/**
 * Returns the call on the top of the call stack. If the call stack is empty then StatusException is thrown.
 *
 * @return Reference to the call on the top of the call stack.
 */
lyric_runtime::CallCell&
lyric_runtime::StackfulCoroutine::currentCallOrThrow()
{
    if (m_callStack.empty()) [[unlikely]]
        throw tempo_utils::StatusException(InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "empty call stack"));
    return m_callStack.back();
}

/**
 * Returns the call on the top of the call stack. If the call stack is empty then StatusException is thrown.
 *
 * @return Const reference to the call on the top of the call stack.
 */
const lyric_runtime::CallCell&
lyric_runtime::StackfulCoroutine::currentCallOrThrow() const
{
    if (m_callStack.empty()) [[unlikely]]
        throw tempo_utils::StatusException(InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "empty call stack"));
    return m_callStack.back();
}

tempo_utils::Status
lyric_runtime::StackfulCoroutine::pushCall(
    const CallCell &value,
    const lyric_object::BytecodeIterator &ip,
    BytecodeSegment *sp)
{
    m_IP = ip;
    m_SP = sp;
    m_callStack.push_back(value);
    return {};
}

tempo_utils::Status
lyric_runtime::StackfulCoroutine::popCall(CallCell &value)
{
    if (m_callStack.empty())
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "call stack is empty");
    value = std::move(m_callStack.back());
    m_callStack.pop_back();
    return {};
}

tempo_utils::Status
lyric_runtime::StackfulCoroutine::peekCall(const CallCell **valueptr, int offset) const
{
    TU_ASSERT (valueptr != nullptr);
    offset = calculate_stack_index(m_callStack, offset);
    if (offset < 0)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid call stack offset");
    auto &value = m_callStack.at(offset);
    *valueptr = &value;
    return {};
}

tempo_utils::Status
lyric_runtime::StackfulCoroutine::peekCall(CallCell **valueptr, int offset)
{
    TU_ASSERT (valueptr != nullptr);
    offset = calculate_stack_index(m_callStack, offset);
    if (offset < 0)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid call stack offset");
    *valueptr = &m_callStack[offset];
    return {};
}

tempo_utils::Status
lyric_runtime::StackfulCoroutine::dropCall(int offset)
{
    offset = calculate_stack_index(m_callStack, offset);
    if (offset < 0)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid call stack offset");
    m_callStack.erase(m_callStack.cbegin() + offset);
    return {};
}

bool
lyric_runtime::StackfulCoroutine::callStackEmpty() const
{
    return m_callStack.empty();
}

int
lyric_runtime::StackfulCoroutine::callStackSize() const
{
    return m_callStack.size();
}

std::vector<lyric_runtime::CallCell>::const_reverse_iterator
lyric_runtime::StackfulCoroutine::callsBegin() const
{
    return m_callStack.crbegin();
}

std::vector<lyric_runtime::CallCell>::const_reverse_iterator
lyric_runtime::StackfulCoroutine::callsEnd() const
{
    return m_callStack.crend();
}

tempo_utils::Status
lyric_runtime::StackfulCoroutine::pushData(const DataCell &value)
{
    m_dataStack.push_back(value);
    return {};
}

tempo_utils::Status
lyric_runtime::StackfulCoroutine::popData(DataCell &value)
{
    if (m_dataStack.empty())
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "data stack is empty");
    value = std::move(m_dataStack.back());
    m_dataStack.pop_back();
    return {};
}

tempo_utils::Status
lyric_runtime::StackfulCoroutine::popData(int count, std::vector<DataCell> &values)
{
    if (count < 0)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "pop data request count cannot be negative");
    if (std::cmp_greater(count, m_dataStack.size()))
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "not enough values on data stack");
    values.resize(count);
    for (int di = 0, si = m_dataStack.size() - count; di < count; di++, si++) {
        values[di] = std::move(m_dataStack[si]);
    }
    m_dataStack.resize(m_dataStack.size() - count);
    return {};
}

tempo_utils::Status
lyric_runtime::StackfulCoroutine::peekData(const DataCell **valueptr, int offset) const
{
    offset = calculate_stack_index(m_dataStack, offset);
    if (offset < 0)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid data stack offset");
    const auto &value = m_dataStack.at(offset);
    *valueptr = &value;
    return {};
}

tempo_utils::Status
lyric_runtime::StackfulCoroutine::peekData(DataCell **valueptr, int offset)
{
    offset = calculate_stack_index(m_dataStack, offset);
    if (offset < 0)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid data stack offset");
    *valueptr = &m_dataStack[offset];
    return {};
}

tempo_utils::Status
lyric_runtime::StackfulCoroutine::dropData(int offset)
{
    offset = calculate_stack_index(m_dataStack, offset);
    if (offset < 0)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid data stack offset");
    m_dataStack.erase(m_dataStack.cbegin() + offset);
    return {};
}

tempo_utils::Status
lyric_runtime::StackfulCoroutine::extendDataStack(int count)
{
    if (count < 0)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "cannot extend data stack using negative count");
    m_dataStack.resize(m_dataStack.size() + count);
    return {};
}

tempo_utils::Status
lyric_runtime::StackfulCoroutine::resizeDataStack(int size)
{
    if (size < 0)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "cannot resize data stack using negative size");
    m_dataStack.resize(size);
    return {};
}

bool
lyric_runtime::StackfulCoroutine::dataStackEmpty() const
{
    return m_dataStack.empty();
}

int
lyric_runtime::StackfulCoroutine::dataStackSize() const
{
    return m_dataStack.size();
}

std::vector<lyric_runtime::DataCell>::const_reverse_iterator
lyric_runtime::StackfulCoroutine::dataBegin() const
{
    return m_dataStack.crbegin();
}

std::vector<lyric_runtime::DataCell>::const_reverse_iterator
lyric_runtime::StackfulCoroutine::dataEnd() const
{
    return m_dataStack.crend();
}

void
lyric_runtime::StackfulCoroutine::pushGuard(int stackGuard)
{
    if (stackGuard < 0)
        stackGuard = callStackSize();
    TU_ASSERT (stackGuard > 0);
    TU_ASSERT (m_guardStack.empty() || m_guardStack.back() < stackGuard);
    m_guardStack.push_back(stackGuard);
}

int
lyric_runtime::StackfulCoroutine::popGuard()
{
    TU_ASSERT (!m_guardStack.empty());
    auto stackGuard = m_guardStack.back();
    m_guardStack.pop_back();
    return stackGuard;
}

int
lyric_runtime::StackfulCoroutine::peekGuard() const
{
    TU_ASSERT (!m_guardStack.empty());
    return m_guardStack.back();
}

bool
lyric_runtime::StackfulCoroutine::checkGuard() const
{
    if (m_guardStack.empty())
        return true;
    return std::cmp_less_equal(m_guardStack.back(), m_callStack.size());
}

int
lyric_runtime::StackfulCoroutine::guardStackSize() const
{
    return m_guardStack.size();
}

/**
 * Resets the coroutine to the initial state. The instruction pointer and segment pointer are cleared,
 * and all information on the call, data, and guard stacks is dropped.
 */
void
lyric_runtime::StackfulCoroutine::reset()
{
    m_IP = {};
    m_SP = nullptr;
    m_callStack.clear();
    m_dataStack.clear();
    m_guardStack.clear();
}
