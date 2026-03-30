
#include <lyric_runtime/connection.h>
#include <tempo_utils/log_message.h>

lyric_runtime::Connection::Connection(
    const tempo_utils::UUID &id,
    std::shared_ptr<AbstractTransport> transport,
    const tempo_utils::Url &nodeUrl)
    : m_id(id),
      m_transport(std::move(transport)),
      m_nodeUrl(nodeUrl),
      m_writer(nullptr),
      m_async(nullptr)
{
    TU_ASSERT (m_id.isValid());
    TU_NOTNULL (m_transport);
}

lyric_runtime::Connection::~Connection()
{
    TU_LOG_WARN_IF(m_writer) << "writer was not detached";
}

tempo_utils::UUID
lyric_runtime::Connection::getId() const
{
    return m_id;
}

tempo_utils::Url
lyric_runtime::Connection::getNodeUrl() const
{
    return m_nodeUrl;
}

tempo_utils::Status
lyric_runtime::Connection::registerConnect(SystemScheduler *systemScheduler, std::shared_ptr<Promise> promise)
{
    TU_NOTNULL (systemScheduler);
    TU_NOTNULL (promise);
    //auto *loop = systemScheduler->systemLoop();
    return m_transport->attach(m_nodeUrl);
}

bool
lyric_runtime::Connection::isAttached()
{
    absl::MutexLock locker(&m_lock);
    return m_writer != nullptr;
}

tempo_utils::Status
lyric_runtime::Connection::attach(AbstractPortWriter *writer)
{
    absl::MutexLock locker(&m_lock);
    m_writer = writer;
    TU_LOG_V << "attached writer " << (void *) writer << " to port " << m_id.toString();
    return {};
}

tempo_utils::Status
lyric_runtime::Connection::detach()
{
    absl::MutexLock locker(&m_lock);
    TU_LOG_V << "detaching writer " << (void *) m_writer << " from port " << m_id.toString();
    m_writer = nullptr;
    return {};
}

bool
lyric_runtime::Connection::hasPending()
{
    absl::MutexLock locker(&m_lock);
    return !m_pending.empty();
}

std::shared_ptr<tempo_utils::ImmutableBytes>
lyric_runtime::Connection::nextPending()
{
    absl::MutexLock locker(&m_lock);
    TU_ASSERT (!m_pending.empty());
    auto payload = m_pending.front();
    m_pending.pop();
    return payload;
}

int
lyric_runtime::Connection::numPending()
{
    absl::MutexLock locker(&m_lock);
    return m_pending.size();
}
void
lyric_runtime::Connection::receive(std::shared_ptr<tempo_utils::ImmutableBytes> payload)
{
    absl::MutexLock locker(&m_lock);

    m_pending.push(std::move(payload));
    if (!m_async)
        return;
    uv_async_send(m_async);
    m_async = nullptr;
}

void
lyric_runtime::Connection::readyToReceive(uv_async_t *async)
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
lyric_runtime::Connection::send(std::shared_ptr<tempo_utils::ImmutableBytes> payload)
{
    absl::MutexLock locker(&m_lock);
    auto status = m_writer->write(std::move(payload));
    TU_ASSERT (status.isOk());
}
