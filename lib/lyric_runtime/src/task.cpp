
#include <lyric_runtime/promise.h>
#include <lyric_runtime/system_scheduler.h>
#include <lyric_runtime/task.h>

lyric_runtime::Task::Task(bool isMainTask, SystemScheduler *scheduler)
    : m_isMainTask(isMainTask),
      m_state(State::Initial),
      m_scheduler(scheduler),
      m_monitor(nullptr),
      m_prev(nullptr),
      m_next(nullptr)
{
    TU_ASSERT (m_scheduler != nullptr);
}

bool
lyric_runtime::Task::isMainTask() const
{
    return m_isMainTask;
}

lyric_runtime::SystemScheduler *
lyric_runtime::Task::getSystemScheduler() const
{
    return m_scheduler;
}

lyric_runtime::StackfulCoroutine *
lyric_runtime::Task::stackfulCoroutine()
{
    return &m_coro;
}

lyric_runtime::Task::State
lyric_runtime::Task::getState() const
{
    return m_state;
}

void
lyric_runtime::Task::setState(State state)
{
    m_state = state;
}

uv_async_t *
lyric_runtime::Task::getMonitor() const
{
    return m_monitor;
}

void
lyric_runtime::Task::setMonitor(uv_async_t *monitor)
{
    TU_ASSERT (m_monitor == nullptr);
    m_monitor = monitor;
}

void
lyric_runtime::Task::signalMonitor()
{
    if (m_monitor) {
        uv_async_send(m_monitor);
        m_monitor = nullptr;
    }
}

lyric_runtime::Task *
lyric_runtime::Task::prevTask() const
{
    return m_prev;
}

lyric_runtime::Task *
lyric_runtime::Task::nextTask() const
{
    return m_next;
}

void
lyric_runtime::Task::attach(Task **head)
{
    TU_ASSERT (head != nullptr);

    // attach task to the end of the queue
    if (*head == nullptr) {
        m_prev = nullptr;
        m_next = nullptr;
        *head = this;
    } else {
        auto *prev = (*head)->m_prev;
        auto *next = (*head)->m_next;
        if (prev == nullptr && next == nullptr) {
            (*head)->m_prev = this;
            (*head)->m_next = this;
            m_prev = *head;
            m_next = *head;
        } else {
            TU_ASSERT (prev != nullptr);
            TU_ASSERT (next != nullptr);
            m_prev = prev;
            m_next = *head;
            prev->m_next = this;
            (*head)->m_prev = this;
        }
    }
}

void
lyric_runtime::Task::detach(Task **head)
{
    TU_ASSERT (head != nullptr);

    auto *prev = m_prev;
    auto *next = m_next;

    if (prev != nullptr && next != nullptr) {
        if (prev == next) {
            next->m_next = nullptr;
            next->m_prev = nullptr;
        } else {
            prev->m_next = m_next;
            next->m_prev = m_prev;
        }
        if (this == *head) {
            *head = next;
        }
    } else {
        TU_ASSERT (prev == nullptr);
        TU_ASSERT (next == nullptr);
        if (this == *head) {
            *head = nullptr;
        }
    }
}

void
lyric_runtime::Task::suspend()
{
    m_scheduler->suspendTask(this);
}

void
lyric_runtime::Task::resume()
{
    m_scheduler->resumeTask(this);
}

void
lyric_runtime::Task::appendPromise(std::shared_ptr<Promise> promise)
{
    TU_ASSERT (promise->getState() == Promise::State::Pending);
    m_promises.push(promise);
}

void
lyric_runtime::Task::adaptPromises(BytecodeInterpreter *interp, InterpreterState *state)
{
    while (!m_promises.empty()) {
        auto promise = m_promises.front();
        TU_ASSERT (promise->getState() == Promise::State::Pending);
        promise->adapt(interp, state);
        TU_ASSERT (promise->getState() == Promise::State::Completed || promise->getState() == Promise::State::Rejected);
        m_promises.pop();
    }
}
