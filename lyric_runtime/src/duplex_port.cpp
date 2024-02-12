
#include <lyric_runtime/duplex_port.h>
#include <tempo_utils/log_message.h>

lyric_runtime::DuplexPort::DuplexPort(const tempo_utils::Url &uri)
    : m_uri(uri),
      m_writer(nullptr),
      m_async(nullptr)
{
}

lyric_runtime::DuplexPort::~DuplexPort()
{
    TU_LOG_WARN_IF(m_writer) << "writer was not detached";
}

tempo_utils::Url
lyric_runtime::DuplexPort::getUrl() const
{
    return m_uri;
}

bool
lyric_runtime::DuplexPort::isAttached()
{
    absl::MutexLock locker(&m_lock);
    return m_writer != nullptr;
}

tempo_utils::Status
lyric_runtime::DuplexPort::attach(AbstractPortWriter *writer)
{
    absl::MutexLock locker(&m_lock);
    m_writer = writer;
    TU_LOG_V << "attached writer " << (void *) writer << " to port " << m_uri;
    return InterpreterStatus::ok();
}

tempo_utils::Status
lyric_runtime::DuplexPort::detach()
{
    absl::MutexLock locker(&m_lock);
    TU_LOG_V << "detaching writer " << (void *) m_writer << " from port " << m_uri;
    m_writer = nullptr;
    return InterpreterStatus::ok();
}

bool
lyric_runtime::DuplexPort::hasPending()
{
    absl::MutexLock locker(&m_lock);
    return !m_pending.empty();
}

lyric_serde::LyricPatchset
lyric_runtime::DuplexPort::nextPending()
{
    absl::MutexLock locker(&m_lock);
    TU_ASSERT (!m_pending.empty());
    auto patchset = m_pending.front();
    m_pending.pop();
    return patchset;
}

int
lyric_runtime::DuplexPort::numPending()
{
    absl::MutexLock locker(&m_lock);
    return m_pending.size();
}
void
lyric_runtime::DuplexPort::receive(const lyric_serde::LyricPatchset &patchset)
{
    absl::MutexLock locker(&m_lock);

    m_pending.push(patchset);
    if (!m_async)
        return;
    uv_async_send(m_async);
    m_async = nullptr;
}

void
lyric_runtime::DuplexPort::readyToReceive(uv_async_t *async)
{
    absl::MutexLock locker(&m_lock);

    TU_ASSERT (m_async == nullptr);
    if (!m_pending.empty()) {
        uv_async_send(async);
    } else {
        m_async = async;
    }
}

void
lyric_runtime::DuplexPort::send(const lyric_serde::LyricPatchset &patchset)
{
    absl::MutexLock locker(&m_lock);
    auto status = m_writer->write(patchset);
    TU_ASSERT (status.isOk());
}
