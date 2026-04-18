
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/promise.h>
#include <lyric_runtime/system_scheduler.h>

void
lyric_runtime::PromiseOperations::onPartial(
    std::vector<std::shared_ptr<Promise>>::const_iterator depsBegin,
    std::vector<std::shared_ptr<Promise>>::const_iterator depsEnd,
    std::shared_ptr<AsyncHandle> &done)
{
    done->sendSignal();
}

lyric_runtime::Promise::Promise(std::unique_ptr<PromiseOperations> ops, Private)
    : m_ops(std::move(ops)),
      m_state(State::Initial),
      m_waiter(nullptr)
{
}

lyric_runtime::Promise::~Promise()
{
}

class AcceptWrapperOps : public lyric_runtime::PromiseOperations {
public:
    explicit AcceptWrapperOps(lyric_runtime::AcceptCallback accept)
        : m_accept(std::move(accept))
    {}

    void onAccept(
        lyric_runtime::Promise *promise,
        const lyric_runtime::Waiter *waiter,
        lyric_runtime::InterpreterState *state) override
    {
        m_accept(promise, waiter, state);
    }

private:
    lyric_runtime::AcceptCallback m_accept;
};

/**
 * Constructs a new promise managed by a shared ptr.
 *
 * @param ops The promise operations implementation.
 * @return The new promise.
 */
std::shared_ptr<lyric_runtime::Promise>
lyric_runtime::Promise::create(std::unique_ptr<PromiseOperations> ops)
{
    return std::make_shared<Promise>(std::move(ops), Private{});
}

/**
 * Constructs a new promise managed by a shared ptr.
 *
 * @param accept Optional accept callback which is invoked when the result becomes available.
 * @return The new promise.
 */
std::shared_ptr<lyric_runtime::Promise>
lyric_runtime::Promise::create(AcceptCallback accept)
{
    std::unique_ptr<PromiseOperations> ops;
    if (accept) {
        ops = std::make_unique<AcceptWrapperOps>(std::move(accept));
    } else {
        ops = std::make_unique<PromiseOperations>();
    }
    return std::make_shared<Promise>(std::move(ops), Private{});
}

/**
 * Constructs a new promise which is already completed and contains the specified `result`.
 *
 * @param result
 * @return The new promise.
 */
std::shared_ptr<lyric_runtime::Promise>
lyric_runtime::Promise::completed(const DataCell &result)
{
    auto promise = create();
    promise->complete(result);
    return promise;
}

/**
 * Constructs a new promise which is already rejected and contains the specified `result`, which is
 * expected to contain a Status.
 *
 * @param result
 * @return The new promise.
 */
std::shared_ptr<lyric_runtime::Promise>
lyric_runtime::Promise::rejected(const DataCell &result)
{
    auto promise = create();
    promise->reject(result);
    return promise;
}

lyric_runtime::Promise::State
lyric_runtime::Promise::getState() const
{
    return m_state;
}

lyric_runtime::DataCell
lyric_runtime::Promise::getResult() const
{
    return m_result;
}

tempo_utils::Status
lyric_runtime::Promise::pending(Waiter *waiter)
{
    TU_NOTNULL (waiter);
    if (m_state != State::Initial)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid promise state");
    TU_ASSERT (m_waiter == nullptr);
    m_waiter = waiter;
    m_state = State::Pending;
    return {};
}

tempo_utils::Status
lyric_runtime::Promise::target(std::shared_ptr<AsyncHandle> async, Waiter *waiter)
{
    TU_NOTNULL (waiter);
    if (m_state != State::Initial)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid promise state");
    TU_ASSERT (m_waiter == nullptr);
    TU_ASSERT (m_async == nullptr);
    m_waiter = waiter;
    m_async = std::move(async);
    m_state = State::Target;
    return {};
}

tempo_utils::Status
lyric_runtime::Promise::forward(std::shared_ptr<Promise> &target)
{
    TU_NOTNULL (target);
    if (target->m_state != State::Target)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid target promise");
    if (!m_target.expired())
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "promise has already been forwarded");
    switch (m_state) {
        case State::Pending:
        case State::Target:
            m_target = target;
            target->m_dependencies.push_back(shared_from_this());
            m_state = State::Forwarded;
            return {};
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
                "invalid promise state");
    }
}

tempo_utils::Status
lyric_runtime::Promise::await(SystemScheduler *systemScheduler)
{
    TU_NOTNULL (systemScheduler);
    if (m_waiter == nullptr)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "promise has no waiter");
    switch (m_state) {
        case State::Pending:
        case State::Target:
            systemScheduler->await(m_waiter);
            m_state = State::Waiting;
            return {};
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
                "invalid promise state");
    }
}

void
lyric_runtime::Promise::accept(const Waiter *waiter, InterpreterState *state)
{
    m_ops->onAccept(this, waiter, state);
}

void
lyric_runtime::Promise::setReachable()
{
    for (const auto &dep : m_dependencies) {
        dep->setReachable();
    }
    m_ops->setReachable();
}

tempo_utils::Status
lyric_runtime::Promise::notify()
{
    auto target = m_target.lock();
    if (target == nullptr)
        return {};
    switch (target->m_state) {
        // cannot notify a promise in initial state
        case State::Initial:
            return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
                "invalid promise state");

        // if target is already finished then do nothing
        case State::Completed:
        case State::Rejected:
            return {};

        // otherwise call onPartial to determine whether target can be resolved
        default: {
            auto begin = target->m_dependencies.cbegin();
            auto end = target->m_dependencies.cend();
            auto &done = target->m_async;
            target->m_ops->onPartial(begin, end, done);
            break;
        }
    }

    return {};
}

tempo_utils::Status
lyric_runtime::Promise::complete(const DataCell &result)
{
    if (!result.isValid())
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid promise result");
    switch (m_state) {
        case State::Initial:
        case State::Pending:
        case State::Target:
        case State::Waiting:
            m_result = result;
            m_state = State::Completed;
            return {};
        case State::Forwarded:
            m_result = result;
            m_state = State::Completed;
            return notify();
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
                "invalid promise state");
    }
}

tempo_utils::Status
lyric_runtime::Promise::reject(const DataCell &result)
{
    if (result.type != DataCellType::STATUS)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid promise result");
    switch (m_state) {
        case State::Initial:
        case State::Pending:
        case State::Target:
        case State::Waiting:
            m_result = result;
            m_state = State::Rejected;
            return {};
        case State::Forwarded:
            m_result = result;
            m_state = State::Rejected;
            return notify();
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
                "invalid promise state");
    }
}