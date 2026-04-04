
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/promise.h>

lyric_runtime::Promise::Promise(std::unique_ptr<PromiseOperations> ops)
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
    return std::make_shared<Promise>(std::move(ops));
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
    return std::make_shared<Promise>(std::move(ops));
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

void
lyric_runtime::Promise::attach(Waiter *waiter)
{
    TU_ASSERT (waiter != nullptr);
    TU_ASSERT (m_state == State::Initial);
    TU_ASSERT (m_waiter == nullptr);
    m_waiter = waiter;
    m_state = State::Pending;
}

void
lyric_runtime::Promise::await(SystemScheduler *systemScheduler)
{
    TU_NOTNULL (systemScheduler);
    TU_NOTNULL (m_waiter);
    TU_ASSERT (m_state == State::Pending);
    systemScheduler->await(m_waiter);
}

void
lyric_runtime::Promise::accept(const Waiter *waiter, InterpreterState *state)
{
    m_ops->onAccept(this, waiter, state);
}

void
lyric_runtime::Promise::adapt(BytecodeInterpreter *interp, InterpreterState *state)
{
    m_ops->onAdapt(this, interp, state);
}

void
lyric_runtime::Promise::setReachable()
{
    m_ops->setReachable();
}

void
lyric_runtime::Promise::complete(const DataCell &result)
{
    TU_ASSERT (result.isValid());
    m_result = result;
    m_state = State::Completed;
}

void
lyric_runtime::Promise::reject(const DataCell &result)
{
    TU_ASSERT (result.type == DataCellType::STATUS);
    m_result = result;
    m_state = State::Rejected;
}