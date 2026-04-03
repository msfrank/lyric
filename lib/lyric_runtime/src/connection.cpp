
#include <queue>

#include <lyric_runtime/connection.h>
#include <lyric_runtime/interpreter_result.h>
#include <tempo_utils/log_message.h>

class lyric_runtime::Connection::Stream : public AbstractStream {
public:
    Stream();

    void connectComplete(AbstractPeer *peer) override;
    void receiveComplete(std::shared_ptr<const tempo_utils::ImmutableBytes> payload) override;
    void remoteError(const tempo_utils::Status &status) override;
    void remoteClose() override;

private:
    struct AbstractData;
    struct InitialData;
    struct ConnectingData;
    struct ActiveData;
    struct DoneData;

    absl::Mutex m_lock;
    ConnectionState m_state ABSL_GUARDED_BY(m_lock);
    std::unique_ptr<AbstractData> m_data ABSL_GUARDED_BY(m_lock);

    ConnectionState getState();
    tempo_utils::Status startConnect(
        std::shared_ptr<AsyncHandle> async,
        Connection *connection,
        std::shared_ptr<AbstractConnectCompleter> completer);
    tempo_utils::Status startReceive(
        std::shared_ptr<AsyncHandle> async,
        std::shared_ptr<Promise> promise,
        std::shared_ptr<AbstractReceiveCompleter> completer);
    tempo_utils::Status sendPayload(std::shared_ptr<const tempo_utils::ImmutableBytes> payload);
    tempo_utils::Status shutdown();

    friend class Connection;
};

struct lyric_runtime::Connection::Stream::AbstractData {
    virtual ~AbstractData() = default;
    virtual void error(const tempo_utils::Status &status) = 0;
    virtual void close() = 0;
};

struct lyric_runtime::Connection::Stream::InitialData : AbstractData {
    void error(const tempo_utils::Status &status) override { TU_LOG_FATAL << status; }
    void close() override {}
};

struct lyric_runtime::Connection::Stream::ConnectingData : AbstractData {
    std::shared_ptr<AsyncHandle> async;
    Connection *conn = nullptr;
    std::shared_ptr<AbstractConnectCompleter> completer;

    void error(const tempo_utils::Status &status) override {
        TU_LOG_ERROR << status;
    }
    void close() override {
        TU_ASSERT (async == nullptr);
        if (conn != nullptr) {
            conn->reset();
        }
    }
};

struct lyric_runtime::Connection::Stream::ActiveData : AbstractData {
    Connection *conn = nullptr;
    AbstractPeer *peer = nullptr;
    std::queue<std::shared_ptr<const tempo_utils::ImmutableBytes>> incoming;
    std::queue<
        std::tuple<std::shared_ptr<AsyncHandle>,std::shared_ptr<Promise>,std::shared_ptr<AbstractReceiveCompleter>>
    > pending;

    void error(const tempo_utils::Status &status) override {
        TU_LOG_ERROR << status;
    }
    void close() override {
        if (peer != nullptr) {
            peer->shutdown();
        }
        if (conn != nullptr) {
            conn->shutdown();
        }
    }
};

struct lyric_runtime::Connection::Stream::DoneData : AbstractData {
    void error(const tempo_utils::Status &status) override { TU_LOG_FATAL << status; }
    void close() override {}
};

lyric_runtime::Connection::Stream::Stream()
    : m_state(ConnectionState::Initial),
      m_data(std::make_unique<InitialData>())
{
}

lyric_runtime::ConnectionState
lyric_runtime::Connection::Stream::getState()
{
    absl::MutexLock lock(&m_lock);
    return m_state;
}

tempo_utils::Status
lyric_runtime::Connection::Stream::startConnect(
    std::shared_ptr<AsyncHandle> async,
    Connection *conn,
    std::shared_ptr<AbstractConnectCompleter> completer)
{
    TU_NOTNULL (async);
    TU_NOTNULL (conn);
    TU_NOTNULL (completer);

    absl::MutexLock lock(&m_lock);
    TU_ASSERT (m_state == ConnectionState::Initial);
    auto next = std::make_unique<ConnectingData>();
    next->async = async;
    next->conn = conn;
    next->completer = std::move(completer);
    m_data = std::move(next);
    m_state = ConnectionState::Connecting;
    return {};
}

void
lyric_runtime::Connection::Stream::connectComplete(AbstractPeer *peer)
{
    TU_NOTNULL (peer);

    absl::MutexLock lock(&m_lock);
    TU_ASSERT (m_state == ConnectionState::Connecting);
    auto prev = std::move(m_data);
    auto *connecting = static_cast<ConnectingData *>(prev.get());
    auto next = std::make_unique<ActiveData>();
    next->peer = peer;
    next->conn = connecting->conn;

    if (!connecting->async->sendSignal()) {
        connecting->completer->error(InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "unexpected error completing connect"));
        connecting->conn->reset();
        peer->reset();
        m_data = std::make_unique<InitialData>();
        m_state = ConnectionState::Initial;
    } else {
        connecting->completer->connectComplete();
        m_data = std::move(next);
        m_state = ConnectionState::Active;
    }
}

tempo_utils::Status
lyric_runtime::Connection::Stream::startReceive(
    std::shared_ptr<AsyncHandle> async,
    std::shared_ptr<Promise> promise,
    std::shared_ptr<AbstractReceiveCompleter> completer)
{
    TU_NOTNULL (async);
    TU_NOTNULL (promise);
    TU_NOTNULL (completer);

    absl::MutexLock lock(&m_lock);
    TU_ASSERT (m_state == ConnectionState::Active);
    auto *active = static_cast<ActiveData *>(m_data.get());
    active->pending.emplace(async, std::move(promise), std::move(completer));
    return {};
}

void
lyric_runtime::Connection::Stream::receiveComplete(std::shared_ptr<const tempo_utils::ImmutableBytes> payload)
{
    TU_NOTNULL (payload);

    absl::MutexLock lock(&m_lock);
    TU_ASSERT (m_state == ConnectionState::Active);
    auto *active = static_cast<ActiveData *>(m_data.get());

    if (!active->pending.empty()) {
        auto &pending = active->pending.front();
        auto &completer = std::get<std::shared_ptr<AbstractReceiveCompleter>>(pending);
        auto async = std::get<std::shared_ptr<AsyncHandle>>(pending);
        if (!async->sendSignal()) {
            completer->error(InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
                "unexpected error completing connect"));
        } else {
            completer->receiveComplete(std::move(payload));
        }
        active->pending.pop();
    } else {
        active->incoming.push(std::move(payload));
    }
}

tempo_utils::Status
lyric_runtime::Connection::Stream::sendPayload(std::shared_ptr<const tempo_utils::ImmutableBytes> payload)
{
    TU_NOTNULL (payload);

    absl::MutexLock lock(&m_lock);
    TU_ASSERT (m_state == ConnectionState::Active);
    auto *active = static_cast<ActiveData *>(m_data.get());
    return active->peer->send(std::move(payload));
}

tempo_utils::Status
lyric_runtime::Connection::Stream::shutdown()
{
    absl::MutexLock lock(&m_lock);
    m_data->close();
    m_data = std::make_unique<DoneData>();
    m_state = ConnectionState::Done;
    return {};
}

void
lyric_runtime::Connection::Stream::remoteError(const tempo_utils::Status &status)
{
    TU_LOG_WARN << status;
}

void
lyric_runtime::Connection::Stream::remoteClose()
{
}

lyric_runtime::Connection::Connection(
    const tempo_utils::UUID &id,
    std::shared_ptr<AbstractTransport> transport,
    const tempo_utils::Url &nodeUrl)
    : m_id(id),
      m_transport(std::move(transport)),
      m_nodeUrl(nodeUrl),
      m_stream(std::make_shared<Stream>())
{
    TU_ASSERT (m_id.isValid());
    TU_NOTNULL (m_transport);
}

lyric_runtime::Connection::~Connection()
{
    TU_LOG_WARN_IF (m_stream) << "connection " << m_id.toString() << "was not shut down cleanly";
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

lyric_runtime::ConnectionState
lyric_runtime::Connection::getState() const
{
    return m_stream->getState();
}

tempo_utils::Status
lyric_runtime::Connection::registerConnect(
    SystemScheduler *systemScheduler,
    std::shared_ptr<Promise> promise,
    std::shared_ptr<AbstractConnectCompleter> completer)
{
    TU_NOTNULL (systemScheduler);
    TU_NOTNULL (promise);

    std::shared_ptr<AsyncHandle> async;
    TU_ASSIGN_OR_RETURN (async, systemScheduler->registerAsync(std::move(promise)));
    m_stream->startConnect(async, this, std::move(completer));

    return m_transport->connect(m_stream, m_nodeUrl);
}

tempo_utils::Status
lyric_runtime::Connection::registerReceive(
    SystemScheduler *systemScheduler,
    std::shared_ptr<Promise> promise,
    std::shared_ptr<AbstractReceiveCompleter> completer)
{
    TU_NOTNULL (systemScheduler);
    TU_NOTNULL (promise);

    std::shared_ptr<AsyncHandle> async;
    TU_ASSIGN_OR_RETURN (async, systemScheduler->registerAsync(promise));
    return m_stream->startReceive(async, promise, std::move(completer));
}

tempo_utils::Status
lyric_runtime::Connection::send(std::shared_ptr<const tempo_utils::ImmutableBytes> payload)
{
    return m_stream->sendPayload(std::move(payload));
}

tempo_utils::Status
lyric_runtime::Connection::shutdown()
{
    return m_stream->shutdown();
}

void
lyric_runtime::Connection::reset()
{
}
