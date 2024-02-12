#include <absl/strings/substitute.h>

#include <lyric_runtime/interpreter_state.h>
#include <lyric_serde/patchset_value.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

#include "string_ref.h"

StringRef::StringRef(const lyric_runtime::VirtualTable *vtable) : BaseRef(vtable)
{
    m_data = nullptr;
    m_size = 0;
}

StringRef::StringRef(const lyric_runtime::VirtualTable *vtable, const UChar *data, int32_t size) : BaseRef(vtable)
{
    TU_ASSERT (vtable != nullptr);
    TU_ASSERT (data != nullptr);
    TU_ASSERT (size >= 0);

    m_data = new char[size];
    m_size = size;
    memcpy(m_data, data, size);
}

StringRef::~StringRef()
{
    TU_LOG_INFO << "free StringRef" << StringRef::toString();
    delete[] m_data;
}

lyric_runtime::DataCell
StringRef::getField(const lyric_runtime::DataCell &field) const
{
    return lyric_runtime::DataCell();
}

lyric_runtime::DataCell
StringRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return lyric_runtime::DataCell();
}

bool
StringRef::equals(const AbstractRef *other) const
{
    auto *other_ = static_cast<const StringRef *>(other);

    if (m_size != other_->m_size)
        return false;
    return !memcmp(m_data, other_->m_data, m_size);
}

bool
StringRef::rawSize(tu_int32 &size) const
{
    size = m_size;
    return true;
}

tu_int32
StringRef::rawCopy(tu_int32 offset, char *dst, tu_int32 size)
{
    if (offset < 0)
        return -1;
    if (dst == nullptr)
        return -1;
    if (size <= 0)
        return -1;

    if (m_size <= offset)
        return 0;

    auto ncopied = size < m_size - offset? size : static_cast<tu_int32>(m_size - offset);
    memcpy(dst, m_data, ncopied);
    return ncopied;
}

bool
StringRef::utf8Value(std::string &utf8) const
{
    utf8 = std::string(m_data, m_size);
    return true;
}

bool
StringRef::hashValue(absl::HashState state)
{
    absl::HashState::combine_contiguous(std::move(state), m_data, m_size);
    return true;
}

bool
StringRef::serializeValue(lyric_serde::PatchsetState &state, tu_uint32 &index)
{
    std::string s;
    if (m_data) {
        s = std::string(m_data, m_size);
    }
    auto appendValueResult = state.appendValue(tempo_utils::AttrValue(s));
    if (appendValueResult.isStatus()) {
        index = lyric_runtime::INVALID_ADDRESS_U32;
        return false;
    }
    auto *value = appendValueResult.getResult();
    index = value->getAddress().getAddress();
    return true;
}

std::string
StringRef::toString() const
{
    std::string s;
    if (m_data) {
        s = std::string(m_data, m_size);
    }
    return absl::Substitute("<$0: StringRef \"$1\">", this, s);
}

lyric_runtime::DataCell
StringRef::stringAt(int index) const
{
    if (m_data == nullptr)
        return lyric_runtime::DataCell::nil();
    UChar32 char32;
    U8_GET_UNSAFE((const tu_uint8 *) m_data, index, char32);
    return lyric_runtime::DataCell(char32);
}

lyric_runtime::DataCell
StringRef::stringCompare(StringRef *other) const
{
    TU_ASSERT (other != nullptr);

    if (m_data == nullptr && other->m_data == nullptr)
        return lyric_runtime::DataCell(static_cast<int64_t>(0));
    if (m_data == nullptr && other->m_data != nullptr)
        return lyric_runtime::DataCell(static_cast<int64_t>(-1));
    if (m_data != nullptr && other->m_data == nullptr)
        return lyric_runtime::DataCell(static_cast<int64_t>(1));

    UCharIterator lhs, rhs;
    uiter_setUTF8(&lhs, m_data, m_size);
    uiter_setUTF8(&rhs, other->m_data, other->m_size);

    auto cmp = u_strCompareIter(&lhs, &rhs, true);
    return lyric_runtime::DataCell(static_cast<int64_t>(cmp));
}

lyric_runtime::DataCell
StringRef::stringLength() const
{
    tu_int32 length = 0;

    if (m_data != nullptr) {
        tu_int32 index = 0;
        while (index < m_size) {
            U8_FWD_1_UNSAFE(m_data, index);
            length++;
        }
    }

    return lyric_runtime::DataCell(static_cast<int64_t>(length));
}

const char *
StringRef::getStringData() const
{
    return m_data;
}

int32_t
StringRef::getStringSize() const
{
    return m_size;
}

void
StringRef::setStringData(const char *data, int32_t size)
{
    delete[] m_data;
    m_data = new char[size];
    m_size = size;
    memcpy(m_data, data, size);
}

void
StringRef::setMembersReachable()
{
}

void
StringRef::clearMembersReachable()
{
}

tempo_utils::Status
string_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<StringRef>(vtable);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
string_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT (frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.type == lyric_runtime::DataCellType::UTF8);
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<StringRef *>(receiver);
    instance->setStringData(arg0.data.utf8.data, arg0.data.utf8.size);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
string_at(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT (frame.numArguments() == 1);
    const auto &index = frame.getArgument(0);
    TU_ASSERT(index.type == lyric_runtime::DataCellType::I64);
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<StringRef *>(receiver);
    currentCoro->pushData(instance->stringAt(index.data.i64));
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
string_compare(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.type == lyric_runtime::DataCellType::REF);
    const auto &arg1 = frame.getArgument(1);
    TU_ASSERT(arg1.type == lyric_runtime::DataCellType::REF);

    auto *lhs = static_cast<StringRef *>(arg0.data.ref);
    auto *rhs = static_cast<StringRef *>(arg1.data.ref);
    currentCoro->pushData(lhs->stringCompare(rhs));
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
string_length(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<StringRef *>(receiver);
    currentCoro->pushData(instance->stringLength());
    return lyric_runtime::InterpreterStatus::ok();
}
