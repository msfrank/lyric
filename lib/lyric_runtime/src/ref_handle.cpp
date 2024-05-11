
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/ref_handle.h>

lyric_runtime::RefHandle::RefHandle()
    : m_handle(nullptr)
{
}

lyric_runtime::RefHandle::RefHandle(std::shared_ptr<InterpreterState> state, void *handle)
    : m_state(std::move(state)),
      m_handle(handle)
{
    TU_ASSERT (m_state != nullptr);
    TU_ASSERT (m_handle != nullptr);
}

lyric_runtime::RefHandle::RefHandle(const RefHandle &other)
    : m_state(other.m_state),
      m_handle(other.m_handle)
{
    if (m_state && m_handle) {
        m_state->m_heap->incrementHandle(m_handle);
    }
}

lyric_runtime::RefHandle::~RefHandle()
{
    if (m_state && m_handle) {
        m_state->m_heap->decrementHandle(m_handle);
    }
}

lyric_runtime::AbstractRef *
lyric_runtime::RefHandle::getRef() const
{
    if (m_state && m_handle) {
        return m_state->m_heap->derefHandle(m_handle);
    }
    return nullptr;
}