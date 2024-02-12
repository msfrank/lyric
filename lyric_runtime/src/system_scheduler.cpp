
#include <lyric_runtime/system_scheduler.h>

lyric_runtime::SystemScheduler::SystemScheduler(uv_loop_t *loop)
    : m_loop(loop),
      m_readyQueue(nullptr),
      m_waitQueue(nullptr),
      m_currentTask(nullptr),
      m_mainTask(nullptr),
      m_waiters(nullptr)
{
    TU_ASSERT (m_loop != nullptr);

    m_mainTask = new Task(TaskType::Main);
    m_mainTask->scheduler = this;
    m_mainTask->state = TaskState::Waiting;

    // attach main task to the end of the wait queue
    m_mainTask->prev = nullptr;
    m_mainTask->next = nullptr;
    m_waitQueue = m_mainTask;
}

lyric_runtime::SystemScheduler::~SystemScheduler()
{
    // move ready tasks to the wait queue
    while (m_readyQueue) {
        suspendTask(m_readyQueue);
    }
    // destroy all suspended tasks
    while (m_waitQueue) {
        destroyTask(m_waitQueue);
    }
    // destroy all pending waiters
    while (m_waiters) {
        destroyWaiter(m_waiters);
    }

    TU_ASSERT (m_readyQueue == nullptr);
    TU_ASSERT (m_waitQueue == nullptr);
    TU_ASSERT (m_waiters == nullptr);
}

uv_loop_t *
lyric_runtime::SystemScheduler::systemLoop() const
{
    return m_loop;
}

lyric_runtime::Task *
lyric_runtime::SystemScheduler::mainTask() const
{
    return m_mainTask;
}

lyric_runtime::StackfulCoroutine *
lyric_runtime::SystemScheduler::mainCoro() const
{
    if (m_mainTask)
        return &m_mainTask->coro;
    return nullptr;
}

lyric_runtime::Task *
lyric_runtime::SystemScheduler::currentTask() const
{
    return m_currentTask;
}

lyric_runtime::StackfulCoroutine *
lyric_runtime::SystemScheduler::currentCoro() const
{
    if (m_currentTask)
        return &m_currentTask->coro;
    return nullptr;
}

lyric_runtime::Task *
lyric_runtime::SystemScheduler::firstReadyTask() const
{
    return m_readyQueue;
}

lyric_runtime::Task *
lyric_runtime::SystemScheduler::firstWaitingTask() const
{
    return m_waitQueue;
}

lyric_runtime::Task *
lyric_runtime::SystemScheduler::selectNextReady()
{
    // if there is no current task, then select the top of the ready queue
    if (m_currentTask == nullptr) {
        m_currentTask = m_readyQueue;
        if (m_currentTask)
            m_currentTask->state = TaskState::Running;
        return m_currentTask;
    }

    // if there is a current task but no other ready tasks, then keep the current task running
    if (m_currentTask->next == nullptr)
        return m_currentTask;

    // if there is a next ready task, then select the next task
    m_currentTask->state = TaskState::Ready;
    auto *selected = m_currentTask->next;
    selected->state = TaskState::Running;
    m_currentTask = selected;
    return m_currentTask;
}

lyric_runtime::Task *
lyric_runtime::SystemScheduler::createTask()
{
    auto *task = new Task(TaskType::Worker);    // task is created in INITIAL state

    task->scheduler = this;

    //
    task->worker = new uv_async_t;
    //uv_async_init(m_loop, task->worker, )

    // update the task state
    task->state = TaskState::Waiting;

    // attach task to the end of the wait queue
    if (m_waitQueue == nullptr) {
        task->prev = nullptr;
        task->next = nullptr;
        m_waitQueue = task;
    } else {
        auto *prev = m_waitQueue->prev;
        auto *next = m_waitQueue->next;
        TU_ASSERT ((prev && next) || (!prev && !next));
        if (prev && next) {
            task->prev = prev;
            task->next = next;
            prev->next = task;
            next->prev = task;
        } else {
            m_waitQueue->prev = task;
            m_waitQueue->next = task;
            task->prev = m_waitQueue;
            task->next = m_waitQueue;
        }
    }

    return task;
}

void
lyric_runtime::SystemScheduler::suspendTask(Task *task)
{
    TU_ASSERT (task != nullptr);
    TU_ASSERT (task->state != TaskState::Initial);

    // if task is already suspended, then do nothing
    if (task->state == TaskState::Waiting)
        return;

    // detach task from the ready queue
    TU_ASSERT ((task->prev && task->next) || (!task->prev && !task->next));
    if (task->prev && task->next) {
        auto *prev = task->prev;
        auto *next = task->next;
        prev->next = task->next;
        next->prev = task->prev;
        if (task == m_readyQueue) {
            m_readyQueue = next;
        }
        if (task == m_currentTask) {
            m_currentTask = nullptr;
        }
    } else {
        if (task == m_readyQueue) {
            m_readyQueue = nullptr;
        }
        if (task == m_currentTask) {
            m_currentTask = nullptr;
        }
    }

    // update the task state
    task->state = TaskState::Waiting;

    // attach task to the end of the wait queue
    if (m_waitQueue == nullptr) {
        task->prev = nullptr;
        task->next = nullptr;
        m_waitQueue = task;
    } else {
        auto *prev = m_waitQueue->prev;
        auto *next = m_waitQueue->next;
        TU_ASSERT ((prev && next) || (!prev && !next));
        if (prev && next) {
            task->prev = prev;
            task->next = next;
            prev->next = task;
            next->prev = task;
        } else {
            m_waitQueue->prev = task;
            m_waitQueue->next = task;
            task->prev = m_waitQueue;
            task->next = m_waitQueue;
        }
    }
}

void
lyric_runtime::SystemScheduler::resumeTask(Task *task)
{
    TU_ASSERT (task != nullptr);
    TU_ASSERT (task->state != TaskState::Initial);

    // if task is ready or running, then do nothing
    if (task->state == TaskState::Running || task->state == TaskState::Ready)
        return;

    // detach task from the wait queue
    TU_ASSERT ((task->prev && task->next) || (!task->prev && !task->next));
    if (task->prev && task->next) {
        auto *prev = task->prev;
        auto *next = task->next;
        prev->next = task->next;
        next->prev = task->prev;
        if (task == m_waitQueue) {
            m_waitQueue = next;
        }
    } else {
        if (task == m_waitQueue) {
            m_waitQueue = nullptr;
        }
    }

    // update the task state
    task->state = TaskState::Ready;

    // attach task to the end of the ready queue
    if (m_readyQueue == nullptr) {
        task->prev = nullptr;
        task->next = nullptr;
        m_readyQueue = task;
    } else {
        auto *prev = m_readyQueue->prev;
        auto *next = m_readyQueue->next;
        TU_ASSERT ((prev && next) || (!prev && !next));
        if (prev && next) {
            task->prev = prev;
            task->next = next;
            prev->next = task;
            next->prev = task;
        } else {
            m_readyQueue->prev = task;
            m_readyQueue->next = task;
            task->prev = m_readyQueue;
            task->next = m_readyQueue;
        }
    }
}

void
lyric_runtime::SystemScheduler::destroyTask(Task *task)
{
    TU_ASSERT (task != nullptr);
    TU_ASSERT (task->state == TaskState::Waiting);

    // detach task from the wait queue
    TU_ASSERT ((task->prev && task->next) || (!task->prev && !task->next));
    if (task->prev && task->next) {
        auto *prev = task->prev;
        auto *next = task->next;
        prev->next = task->next;
        next->prev = task->prev;
        if (task == m_waitQueue) {
            m_waitQueue = next;
        }
    } else {
        if (task == m_waitQueue) {
            m_waitQueue = nullptr;
        }
    }

    delete task;
}

static void
on_timer_complete(uv_timer_t *timer)
{
    auto *waiter = (lyric_runtime::Waiter *) timer->data;

    // if there is a task associated with the waiter, then resume the suspended task.
    // we do this before running the waiter callback so that there will be a coroutine
    // available to the callback.
    if (waiter->task) {
        waiter->task->scheduler->resumeTask(waiter->task);
    }

    // if the waiter has a callback, then run the waiter callback
    if (waiter->cb) {
        waiter->cb(waiter, waiter->data);
    }

    // free the completed waiter
    waiter->task->scheduler->destroyWaiter(waiter);
}

lyric_runtime::Waiter *
lyric_runtime::SystemScheduler::registerTimer(uint64_t timeout, WaiterCallback cb, void *data)
{
    uv_timer_t *timer = (uv_timer_t *) malloc(sizeof(uv_timer_t));
    uv_timer_init(m_loop, timer);

    auto *waiter = new Waiter(m_currentTask);
    waiter->handle = (uv_handle_t *) timer;
    waiter->cb = cb;
    waiter->data = data;

    timer->data = waiter;
    uv_timer_start(timer, on_timer_complete, timeout, 0);

    // attach handle to the end of the handles list
    if (m_waiters == nullptr) {
        waiter->prev = nullptr;
        waiter->next = nullptr;
        m_waiters = waiter;
    } else {
        auto *prev = m_waiters->prev;
        prev->next = waiter;
        waiter->prev = prev;
        waiter->next = m_waiters;
        m_waiters->prev = waiter;
    }

    return waiter;
}

static void
on_async_complete(uv_async_t *async)
{
    auto *waiter = (lyric_runtime::Waiter *) async->data;

    // if there is a task associated with the waiter, then resume the suspended task.
    // we do this before running the waiter callback so that there will be a coroutine
    // available to the callback.
    if (waiter->task) {
        waiter->task->scheduler->resumeTask(waiter->task);
    }

    // if the waiter has a callback, then run the waiter callback
    if (waiter->cb) {
        waiter->cb(waiter, waiter->data);
    }

    // free the completed waiter
    waiter->task->scheduler->destroyWaiter(waiter);
}

lyric_runtime::Waiter *
lyric_runtime::SystemScheduler::registerAsync(uv_async_t **asyncptr, WaiterCallback cb, void *data)
{
    TU_ASSERT (asyncptr != nullptr);

    uv_async_t *async = (uv_async_t *) malloc(sizeof(uv_async_t));
    uv_async_init(m_loop, async, on_async_complete);

    auto *waiter = new Waiter(m_currentTask);
    waiter->handle = (uv_handle_t *) async;
    waiter->cb = cb;
    waiter->data = data;

    async->data = waiter;

    // attach handle to the end of the handles list
    if (m_waiters == nullptr) {
        waiter->prev = nullptr;
        waiter->next = nullptr;
        m_waiters = waiter;
    } else {
        auto *prev = m_waiters->prev;
        prev->next = waiter;
        waiter->prev = prev;
        waiter->next = m_waiters;
        m_waiters->prev = waiter;
    }

    *asyncptr = async;

    return waiter;
}

static void
on_handle_close(uv_handle_t *handle)
{
    free(handle);
}

void
lyric_runtime::SystemScheduler::destroyWaiter(Waiter *waiter)
{
    // close the handle, deferring resource deallocation to the specified callback
    uv_close(waiter->handle, on_handle_close);

    // detach task from the wait queue
    TU_ASSERT ((waiter->prev && waiter->next) || (!waiter->prev && !waiter->next));
    if (waiter->prev && waiter->next) {
        auto *prev = waiter->prev;
        auto *next = waiter->next;
        prev->next = waiter->next;
        next->prev = waiter->prev;
        if (waiter == m_waiters) {
            m_waiters = next;
        }
    } else {
        if (waiter == m_waiters) {
            m_waiters = nullptr;
        }
    }

    delete waiter;
}

bool
lyric_runtime::SystemScheduler::poll()
{
    // TODO: return InterpreterStatus instead of bool
    auto result = uv_run(m_loop, UV_RUN_NOWAIT);
    return result == 0;
}

bool
lyric_runtime::SystemScheduler::blockingPoll()
{
    // TODO: return InterpreterStatus instead of bool
    TU_ASSERT (m_readyQueue == nullptr);
    if (m_waitQueue == nullptr)
        return false;
    uv_run(m_loop, UV_RUN_ONCE);
    return true;
}