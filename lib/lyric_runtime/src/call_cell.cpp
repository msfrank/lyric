#include <absl/strings/substitute.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/call_cell.h>
#include <tempo_utils/log_stream.h>

lyric_runtime::CallCell::CallCell(
    tu_uint32 callIndex,
    tu_uint32 callSegment,
    tu_uint32 procOffset,
    tu_uint32 returnSegment,
    lyric_object::BytecodeIterator returnIP,
    bool returnsValue,
    int stackGuard,
    tu_uint16 numArguments,
    tu_uint16 numRest,
    tu_uint16 numLocals,
    tu_uint16 numLexicals,
    const std::vector<DataCell> &data,
    const VirtualTable *vtable)
    : m_callIndex(callIndex),
      m_callSegment(callSegment),
      m_procOffset(procOffset),
      m_returnSegment(returnSegment),
      m_returnIP(returnIP),
      m_returnsValue(returnsValue),
      m_stackGuard(stackGuard),
      m_numArguments(numArguments),
      m_numRest(numRest),
      m_numLocals(numLocals),
      m_numLexicals(numLexicals),
      m_data(data),
      m_vtable(vtable)
{
    TU_ASSERT (m_vtable != nullptr);
    m_data.resize(numArguments + numRest + numLocals + numLexicals);
}

lyric_runtime::CallCell::CallCell(
    tu_uint32 callIndex,
    tu_uint32 callSegment,
    tu_uint32 procOffset,
    tu_uint32 returnSegment,
    lyric_object::BytecodeIterator returnIP,
    bool returnsValue,
    int stackGuard,
    tu_uint16 numArguments,
    tu_uint16 numRest,
    tu_uint16 numLocals,
    tu_uint16 numLexicals,
    const std::vector<DataCell> &data,
    DataCell receiver)
    : m_callIndex(callIndex),
      m_callSegment(callSegment),
      m_procOffset(procOffset),
      m_returnSegment(returnSegment),
      m_returnIP(returnIP),
      m_returnsValue(returnsValue),
      m_stackGuard(stackGuard),
      m_numArguments(numArguments),
      m_numRest(numRest),
      m_numLocals(numLocals),
      m_numLexicals(numLexicals),
      m_data(data),
      m_receiver(std::move(receiver)),
      m_vtable(nullptr)
{
    TU_ASSERT (m_receiver.type != DataCellType::INVALID);
    m_data.resize(numArguments + numRest + numLocals + numLexicals);
}

lyric_runtime::CallCell::CallCell(
    tu_uint32 callIndex,
    tu_uint32 callSegment,
    tu_uint32 procOffset,
    tu_uint32 returnSegment,
    lyric_object::BytecodeIterator returnIP,
    bool returnsValue,
    int stackGuard,
    tu_uint16 numArguments,
    tu_uint16 numRest,
    tu_uint16 numLocals,
    tu_uint16 numLexicals,
    const std::vector<DataCell> &data)
    : m_callIndex(callIndex),
      m_callSegment(callSegment),
      m_procOffset(procOffset),
      m_returnSegment(returnSegment),
      m_returnIP(returnIP),
      m_returnsValue(returnsValue),
      m_stackGuard(stackGuard),
      m_numArguments(numArguments),
      m_numRest(numRest),
      m_numLocals(numLocals),
      m_numLexicals(numLexicals),
      m_data(data),
      m_vtable(nullptr)
{
    m_data.resize(numArguments + numRest + numLocals + numLexicals);
}

lyric_runtime::CallCell::CallCell(const CallCell &other)
    : m_callIndex(other.m_callIndex),
      m_callSegment(other.m_callSegment),
      m_procOffset(other.m_procOffset),
      m_returnSegment(other.m_returnSegment),
      m_returnIP(other.m_returnIP),
      m_returnsValue(other.m_returnsValue),
      m_stackGuard(other.m_stackGuard),
      m_numArguments(other.m_numArguments),
      m_numRest(other.m_numRest),
      m_numLocals(other.m_numLocals),
      m_numLexicals(other.m_numLexicals),
      m_data(other.m_data),
      m_receiver(other.m_receiver),
      m_vtable(other.m_vtable)
{
}

lyric_runtime::CallCell::CallCell(CallCell &&other) noexcept
{
    // swap data to this instance
    m_callIndex = other.m_callIndex;
    m_callSegment = other.m_callSegment;
    m_procOffset = other.m_procOffset;
    m_returnSegment = other.m_returnSegment;
    m_returnIP = other.m_returnIP;
    m_returnsValue = other.m_returnsValue;
    m_stackGuard = other.m_stackGuard;
    m_numArguments = other.m_numArguments;
    m_numRest = other.m_numRest;
    m_numLocals = other.m_numLocals;
    m_numLexicals = other.m_numLexicals;
    m_data.swap(other.m_data);
    m_receiver = other.m_receiver;
    m_vtable = other.m_vtable;

    // invalidate other instance
    other.m_callIndex = 0;
    other.m_callSegment = 0;
    other.m_procOffset = 0;
    other.m_returnSegment = 0;
    other.m_returnIP = {};
    other.m_stackGuard = 0;
    other.m_numArguments = 0;
    other.m_numRest = 0;
    other.m_numLocals = 0;
    other.m_numLexicals = 0;
    other.m_data.clear();
    other.m_receiver = {};
    other.m_vtable = nullptr;
}

lyric_runtime::CallCell&
lyric_runtime::CallCell::operator=(const CallCell &other)
{
    // copy data to this instance
    m_callIndex = other.m_callIndex;
    m_callSegment = other.m_callSegment;
    m_procOffset = other.m_procOffset;
    m_returnSegment = other.m_returnSegment;
    m_returnIP = other.m_returnIP;
    m_returnsValue = other.m_returnsValue;
    m_stackGuard = other.m_stackGuard;
    m_numArguments = other.m_numArguments;
    m_numRest = other.m_numRest;
    m_numLocals = other.m_numLocals;
    m_numLexicals = other.m_numLexicals;
    m_data = other.m_data;
    m_receiver = other.m_receiver;
    m_vtable = other.m_vtable;
    return *this;
}

lyric_runtime::CallCell&
lyric_runtime::CallCell::operator=(CallCell &&other) noexcept
{
    if (this != &other) {

        // swap data to this instance
        m_callIndex = other.m_callIndex;
        m_callSegment = other.m_callSegment;
        m_procOffset = other.m_procOffset;
        m_returnSegment = other.m_returnSegment;
        m_returnIP = other.m_returnIP;
        m_returnsValue = other.m_returnsValue;
        m_stackGuard = other.m_stackGuard;
        m_numArguments = other.m_numArguments;
        m_numRest = other.m_numRest;
        m_numLocals = other.m_numLocals;
        m_numLexicals = other.m_numLexicals;
        m_data.swap(other.m_data);
        m_receiver = other.m_receiver;
        m_vtable = other.m_vtable;

        // invalidate other instance
        other.m_callIndex = 0;
        other.m_callSegment = 0;
        other.m_procOffset = 0;
        other.m_returnSegment = 0;
        other.m_returnIP = {};
        other.m_stackGuard = 0;
        other.m_numArguments = 0;
        other.m_numRest = 0;
        other.m_numLocals = 0;
        other.m_numLexicals = 0;
        other.m_data.clear();
        other.m_receiver = {};
        other.m_vtable = nullptr;
    }

    return *this;
}

tu_uint32
lyric_runtime::CallCell::getCallIndex() const
{
    return m_callIndex;
}

tu_uint32
lyric_runtime::CallCell::getCallSegment() const
{
    return m_callSegment;
}

tu_uint32
lyric_runtime::CallCell::getProcOffset() const
{
    return m_procOffset;
}

tu_uint32
lyric_runtime::CallCell::getReturnSegment() const
{
    return m_returnSegment;
}

lyric_object::BytecodeIterator
lyric_runtime::CallCell::getReturnIP() const
{
    return m_returnIP;
}

bool
lyric_runtime::CallCell::returnsValue() const
{
    return m_returnsValue;
}

int
lyric_runtime::CallCell::getStackGuard() const
{
    return m_stackGuard;
}

lyric_runtime::DataCell
lyric_runtime::CallCell::getReceiver() const
{
    return m_receiver;
}

const lyric_runtime::VirtualTable *
lyric_runtime::CallCell::getVirtualTable() const
{
    return m_vtable;
}

lyric_runtime::DataCell
lyric_runtime::CallCell::getArgument(int index) const
{
    if (0 <= index && index < m_numArguments)
        return m_data[index];
    return DataCell();
}

void
lyric_runtime::CallCell::setArgument(int index, const DataCell &cell)
{
    if (0 <= index && index < m_numArguments)
        m_data[index] = cell;
}

tu_uint16
lyric_runtime::CallCell::numArguments() const
{
    return m_numArguments;
}

lyric_runtime::DataCell
lyric_runtime::CallCell::getRest(int index) const
{
    if (0 <= index && index < m_numRest)
        return m_data[m_numArguments + index];
    return DataCell();
}

void
lyric_runtime::CallCell::setRest(int index, const DataCell &cell)
{
    if (0 <= index && index < m_numRest)
        m_data[m_numArguments + index] = cell;
}

tu_uint16
lyric_runtime::CallCell::numRest() const
{
    return m_numRest;
}

lyric_runtime::DataCell
lyric_runtime::CallCell::getLocal(int index) const
{
    if (0 <= index && index < m_numLocals)
        return m_data[m_numArguments + m_numRest + index];
    return DataCell();
}

void
lyric_runtime::CallCell::setLocal(int index, const DataCell &cell)
{
    if (0 <= index && index < m_numLocals)
        m_data[m_numArguments + m_numRest + index] = cell;
}

tu_uint16
lyric_runtime::CallCell::numLocals() const
{
    return m_numLocals;
}

lyric_runtime::DataCell
lyric_runtime::CallCell::getLexical(int index) const
{
    if (0 <= index && index < m_numLexicals)
        return m_data[m_numArguments + m_numRest + m_numLocals + index];
    return DataCell();
}

void
lyric_runtime::CallCell::setLexical(int index, const DataCell &cell)
{
    if (0 <= index && index < m_numLexicals)
        m_data[m_numArguments + m_numRest + m_numLocals + index] = cell;
}

tu_uint16
lyric_runtime::CallCell::numLexicals() const
{
    return m_numLexicals;
}

std::string
lyric_runtime::CallCell::toString() const
{
    auto receiver = getReceiver();
    if (receiver.isValid()) {
        return absl::Substitute("CallCell(segment=$0, call=$1 offset=$2, receiver=$3)",
            getCallSegment(),
            getCallIndex(),
            getProcOffset(),
            receiver.toString());
    } else {
        return absl::Substitute("CallCell(segment=$0, call=$1 offset=$2)",
            getCallSegment(),
            getCallIndex(),
            getProcOffset());
    }
}

tempo_utils::LogMessage&&
lyric_runtime::operator<<(tempo_utils::LogMessage &&message, const lyric_runtime::CallCell &cell)
{
    std::forward<tempo_utils::LogMessage>(message) << cell.toString();
    return std::move(message);
}
