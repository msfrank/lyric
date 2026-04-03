
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/promise.h>

lyric_runtime::Promise::Promise(AcceptCallback accept, const PromiseOptions &options)
    : m_accept(accept),
      m_options(options),
      m_state(State::Initial),
      m_waiter(nullptr)
{
}

lyric_runtime::Promise::~Promise()
{
    if (m_options.release) {
        m_options.release(m_options.data);
    }
}

/**
 * Constructs a new promise managed by a shared ptr.
 *
 * @param accept Optional accept callback which is invoked when the result becomes available.
 * @param options Promise options.
 * @return
 */
std::shared_ptr<lyric_runtime::Promise>
lyric_runtime::Promise::create(AcceptCallback accept, const PromiseOptions &options)
{
    return std::make_shared<Promise>(accept, options);
}

/**
 * Constructs a new promise which is already completed and contains the specified `result`.
 *
 * @param result
 * @return
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
 * @return
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

void *
lyric_runtime::Promise::getData() const
{
    return m_options.data;
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
    if (m_accept) {
        m_accept(this, waiter, state);
    }
}

void
lyric_runtime::Promise::adapt(BytecodeInterpreter *interp, InterpreterState *state)
{
    if (m_options.adapt) {
        m_options.adapt(this, interp, state);
    }
}

bool
lyric_runtime::Promise::needsAdapt() const
{
    return m_options.adapt != nullptr;
}

void
lyric_runtime::Promise::setReachable()
{
    if (m_options.reachable) {
        m_options.reachable(m_options.data);
    }
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