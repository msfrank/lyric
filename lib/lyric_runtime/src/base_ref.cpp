
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

const lyric_runtime::AbstractMemberResolver *
lyric_runtime::BaseRef::getMemberResolver() const
{
    return m_vtable;
}

const lyric_runtime::AbstractMethodResolver *
lyric_runtime::BaseRef::getMethodResolver() const
{
    return m_vtable;
}

const lyric_runtime::AbstractExtensionResolver *
lyric_runtime::BaseRef::getExtensionResolver() const
{
    return m_vtable;
}

lyric_common::SymbolUrl
lyric_runtime::BaseRef::getSymbolUrl() const
{
    return m_vtable->getSymbolUrl();
}

lyric_runtime::DataCell
lyric_runtime::BaseRef::getField(const DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
lyric_runtime::BaseRef::setField(const DataCell &field, const DataCell &value)
{
    return {};
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
lyric_runtime::BaseRef::prepareFuture(std::shared_ptr<Promise> promise)
{
    return false;
}

bool
lyric_runtime::BaseRef::awaitFuture(SystemScheduler *systemScheduler)
{
    return false;
}

bool
lyric_runtime::BaseRef::resolveFuture(DataCell &result)
{
    return false;
}

bool
lyric_runtime::BaseRef::applyClosure(Task *task, std::vector<DataCell> &args, lyric_runtime::InterpreterState *state)
{
    return false;
}

tempo_utils::StatusCode
lyric_runtime::BaseRef::errorStatusCode()
{
    return tempo_utils::StatusCode::kOk;
}

std::string
lyric_runtime::BaseRef::errorMessage()
{
    return {};
}

bool
lyric_runtime::BaseRef::isReachable() const
{
    return m_reachable;
}

void
lyric_runtime::BaseRef::setMembersReachable()
{
}

void
lyric_runtime::BaseRef::clearMembersReachable()
{
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
