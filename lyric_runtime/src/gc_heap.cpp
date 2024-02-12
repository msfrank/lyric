
#include <lyric_runtime/gc_heap.h>
#include <tempo_utils/log_stream.h>

lyric_runtime::GCHeap::GCHeap()
    : m_handles(nullptr)
{
}

lyric_runtime::GCHeap::~GCHeap()
{
    for (AbstractRef *instance : m_instances) {
        if (instance == nullptr)
            continue;
        TU_LOG_WARN << "found unreleased heap reference " << (void *) instance;
    }
}

uint32_t
lyric_runtime::GCHeap::insertInstance(AbstractRef *obj)
{
    auto offset = static_cast<uint32_t>(m_instances.size());
    m_instances.push_back(obj);
    return offset;
}

void
lyric_runtime::GCHeap::clearReachable()
{
    for (auto &instance : m_instances) {
        if (instance != nullptr) {
            instance->clearReachable();
        }
    }
}

void
lyric_runtime::GCHeap::deleteUnreachable()
{
    // mark all handles as reachable
    for (HandlePriv *handle = m_handles; handle != nullptr; handle = handle->next) {
        handle->instance->setReachable();
    }
    // deallocate all unreachable instances
    for (tu_uint32 i = 0; i < m_instances.size(); i++) {
        auto *instance = m_instances[i];
        if (instance != nullptr && !instance->isReachable()) {
            delete instance;
            m_instances[i] = nullptr;
        }
    }
}

void *
lyric_runtime::GCHeap::createHandle(AbstractRef *instance)
{
    HandlePriv *handle = nullptr;
    if (m_handles == nullptr) {
        handle = new HandlePriv{ instance, 1, nullptr, nullptr};
        m_handles = handle;
    } else {
        auto *prev = m_handles->prev;
        auto *next = m_handles;
        handle = new HandlePriv{ instance, 1, prev, next};
        prev->next = handle;
        next->prev = handle;
    }
    return handle;
}

void
lyric_runtime::GCHeap::incrementHandle(void *priv)
{
    TU_ASSERT (priv != nullptr);
    auto *handle = static_cast<HandlePriv *>(priv);
    TU_ASSERT (handle->refcount > 0);
    handle->refcount++;
}

void
lyric_runtime::GCHeap::decrementHandle(void *priv)
{
    TU_ASSERT (priv != nullptr);
    auto *handle = static_cast<HandlePriv *>(priv);
    TU_ASSERT (handle->refcount > 0);
    handle->refcount--;
    if (handle->refcount == 0) {
        auto *prev = handle->prev;
        auto *next = handle->next;
        if (prev == nullptr) {
            TU_ASSERT (next == nullptr);
            m_handles = nullptr;
        } else {
            prev->next = next;
            next->prev = prev;
        }
        if (handle == m_handles) {
            m_handles = next;
        }
        delete handle;
    }
}

lyric_runtime::AbstractRef *
lyric_runtime::GCHeap::derefHandle(void *priv)
{
    TU_ASSERT (priv != nullptr);
    auto *handle = static_cast<HandlePriv *>(priv);
    TU_ASSERT (handle->refcount > 0);
    return handle->instance;
}

std::shared_ptr<lyric_runtime::GCHeap>
lyric_runtime::GCHeap::create()
{
    return std::shared_ptr<GCHeap>(new GCHeap());
}
