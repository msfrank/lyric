
#include <lyric_runtime/stackful_coroutine.h>

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
    m_SP = sp;
}

void
lyric_runtime::StackfulCoroutine::pushCall(
    const CallCell &value,
    const lyric_object::BytecodeIterator &ip,
    BytecodeSegment *sp)
{
    m_IP = ip;
    m_SP = sp;
    m_callStack.push_back(value);
}

lyric_runtime::CallCell
lyric_runtime::StackfulCoroutine::popCall()
{
    auto call = m_callStack.back();
    m_callStack.pop_back();
    return call;
}

lyric_runtime::CallCell&
lyric_runtime::StackfulCoroutine::peekCall(int offset)
{
    if (offset < 0)
        return m_callStack[m_callStack.size() + offset];
    return m_callStack[offset];
}

const lyric_runtime::CallCell&
lyric_runtime::StackfulCoroutine::peekCall(int offset) const
{
    if (offset < 0)
        return m_callStack[m_callStack.size() + offset];
    return m_callStack[offset];
}

void
lyric_runtime::StackfulCoroutine::dropCall(int offset)
{
    int index = offset < 0? m_callStack.size() + offset : offset;
    if (0 <= index && std::cmp_less(index, m_callStack.size())) {
        m_callStack.erase(m_callStack.cbegin() + index);
    } else {
        TU_LOG_ERROR << "dropCall failed due to invalid offset " << offset;
    }
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

void
lyric_runtime::StackfulCoroutine::pushData(const DataCell &value)
{
    m_dataStack.push_back(value);
}

lyric_runtime::DataCell
lyric_runtime::StackfulCoroutine::popData()
{
    auto value = m_dataStack.back();
    m_dataStack.pop_back();
    return value;
}

std::vector<lyric_runtime::DataCell>
lyric_runtime::StackfulCoroutine::popData(int count)
{
    if (std::cmp_greater(count, m_dataStack.size()))
        return {};
    std::vector<DataCell> values(count);
    for (int di = 0, si = m_dataStack.size() - count; di < count; di++, si++) {
        values[di] = m_dataStack[si];
    }
    m_dataStack.resize(m_dataStack.size() - count);
    return values;
}

lyric_runtime::DataCell&
lyric_runtime::StackfulCoroutine::peekData(int offset)
{
    if (offset < 0)
        return m_dataStack[m_dataStack.size() + offset];
    return m_dataStack[offset];
}

const lyric_runtime::DataCell&
lyric_runtime::StackfulCoroutine::peekData(int offset) const
{
    if (offset < 0)
        return m_dataStack[m_dataStack.size() + offset];
    return m_dataStack[offset];
}

void
lyric_runtime::StackfulCoroutine::dropData(int offset)
{
    int index = offset < 0? m_dataStack.size() + offset : offset;
    if (0 <= index && std::cmp_less(index, m_dataStack.size())) {
        m_dataStack.erase(m_dataStack.cbegin() + index);
    } else {
        TU_LOG_ERROR << "dropValue failed due to invalid offset " << offset;
    }
}

int
lyric_runtime::StackfulCoroutine::dataStackSize() const
{
    return m_dataStack.size();
}

void
lyric_runtime::StackfulCoroutine::extendDataStack(int count)
{
    TU_ASSERT(count >= 0);
    m_dataStack.resize(m_dataStack.size() + count);
}

void
lyric_runtime::StackfulCoroutine::resizeDataStack(int count)
{
    m_dataStack.resize(count);
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
