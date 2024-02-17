
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/gc_heap.h>
#include <tempo_utils/log_stream.h>

lyric_runtime::BaseRef::BaseRef(const VirtualTable *vtable)
    : m_vtable(vtable)
{
    m_heap = nullptr;
    m_offset = 0;
    m_reachable = false;
}

const lyric_runtime::VirtualTable *
lyric_runtime::BaseRef::getVirtualTable() const
{
    return m_vtable;
}

bool
lyric_runtime::BaseRef::equals(const AbstractRef *other) const
{
    return false;
}

bool
lyric_runtime::BaseRef::rawSize(tu_int32 &size) const
{
    return false;
}

tu_int32
lyric_runtime::BaseRef::rawCopy(tu_int32 offset, char *dst, tu_int32 size)
{
    return -1;
}

bool
lyric_runtime::BaseRef::utf8Value(std::string &utf8) const
{
    return false;
}

bool
lyric_runtime::BaseRef::uriValue(tempo_utils::Url &url) const
{
    return false;
}

bool
lyric_runtime::BaseRef::hashValue(absl::HashState state)
{
    return false;
}

bool
lyric_runtime::BaseRef::serializeValue(lyric_serde::PatchsetState &state, tu_uint32 &index)
{
    return false;
}

bool
lyric_runtime::BaseRef::iteratorValid()
{
    return false;
}

bool
lyric_runtime::BaseRef::iteratorNext(DataCell &cell)
{
    return false;
}

bool
lyric_runtime::BaseRef::attachWaiter(Waiter *waiter)
{
    return false;
}

bool
lyric_runtime::BaseRef::releaseWaiter(Waiter **waiter)
{
    return false;
}

bool
lyric_runtime::BaseRef::resolveFuture(DataCell &result, BytecodeInterpreter *interp, InterpreterState *state)
{
    return false;
}

bool
lyric_runtime::BaseRef::isReachable() const
{
    return m_reachable;
}

void
lyric_runtime::BaseRef::setReachable()
{
    m_reachable = true;
    setMembersReachable();
}

void
lyric_runtime::BaseRef::clearReachable()
{
    m_reachable = false;
    clearMembersReachable();
}

void
lyric_runtime::BaseRef::finalize()
{
}

bool
lyric_runtime::BaseRef::isAttached() const
{
    return m_heap != nullptr;
}

void
lyric_runtime::BaseRef::attach(AbstractHeap *heap, uint32_t offset)
{
    TU_ASSERT (!isAttached());
    m_heap = heap;
    m_offset = offset;
}

void
lyric_runtime::BaseRef::detach()
{
    TU_ASSERT (isAttached());
    m_heap = nullptr;
    m_offset = 0;
}
