
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

    m_mainTask = new Task(TaskType::Main, this);
    m_mainTask->setTaskState(TaskState::Waiting);

    // attach main task to the end of the wait queue
    m_mainTask->attach(&m_waitQueue);
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
        return m_mainTask->stackfulCoroutine();
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
        return m_currentTask->stackfulCoroutine();
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
        if (m_currentTask) {
            m_currentTask->setTaskState(TaskState::Running);
        }
        return m_currentTask;
    }

    // if there is a current task but no other ready tasks, then keep the current task running
    if (m_currentTask->nextTask() == nullptr)
        return m_currentTask;

    // otherwise switch current task state to ready
    m_currentTask->setTaskState(TaskState::Ready);

    // then select the next task
    auto *selected = m_currentTask->nextTask();
    selected->setTaskState(TaskState::Running);
    m_currentTask = selected;
    return m_currentTask;
}

lyric_runtime::Task *
lyric_runtime::SystemScheduler::createTask()
{
    //
    auto *worker = new uv_async_t;
    //uv_async_init(m_loop, task->worker, )

    // create new worker task and attach it to the end of the wait queue
    auto *task = new Task(TaskType::Worker, this, worker);
    task->setTaskState(TaskState::Waiting);
    task->attach(&m_waitQueue);

    return task;
}

void
lyric_runtime::SystemScheduler::suspendTask(Task *task)
{
    TU_ASSERT (task != nullptr);

    switch (task->getTaskState()) {
        case TaskState::Waiting:
            return;     // if task is already suspended, then do nothing
        case TaskState::Ready:
        case TaskState::Running:
            break;
        default:
            TU_LOG_FATAL << "task " << task << " cannot be suspended; unexpected state";
            return;
    }

    // detach task from the ready queue
    task->detach(&m_readyQueue);

    // if task is running then clear the current task
    if (task == m_currentTask) {
        m_currentTask = nullptr;
    }

    // set the task state to Waiting
    task->setTaskState(TaskState::Waiting);

    // attach task to the end of the wait queue
    task->attach(&m_waitQueue);
}

void
lyric_runtime::SystemScheduler::resumeTask(Task *task)
{
    TU_ASSERT (task != nullptr);

    switch (task->getTaskState()) {
        case TaskState::Ready:
        case TaskState::Running:
            return;     // if task is ready or running, then do nothing
        case TaskState::Waiting:
            break;
        default:
            TU_LOG_FATAL << "task " << task << " cannot be resumed; unexpected state";
            return;
    }

    // detach task from the wait queue
    task->detach(&m_waitQueue);

    // set the task state to Ready
    task->setTaskState(TaskState::Ready);

    // attach task to the end of the ready queue
    task->attach(&m_readyQueue);
}

void
lyric_runtime::SystemScheduler::destroyTask(Task *task)
{
    TU_ASSERT (task != nullptr);
    TU_ASSERT (task->getTaskState() == TaskState::Waiting);

    // detach task from the wait queue and deallocate it
    task->detach(&m_waitQueue);
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
        waiter->task->resume();
    }

    // if the waiter has an attached promise, then run the accept callback
    if (waiter->promise) {
        auto promise = waiter->promise;
        promise->accept();

        // if the promise has an adapt callback, then schedule it to be called before the task resumes
        if (promise->needsAdapt()) {
            auto *task = waiter->task;
            task->appendPromise(promise);
        }
    }

    // free the completed waiter
    auto *scheduler = waiter->task->getSystemScheduler();
    scheduler->destroyWaiter(waiter);
}

void
lyric_runtime::SystemScheduler::registerTimer(tu_uint64 deadline, std::shared_ptr<Promise> promise)
{
    TU_ASSERT (promise != nullptr);
    TU_ASSERT (promise->getPromiseState() == PromiseState::Initial);

    // initialize the timer handle
    auto *timer = (uv_timer_t *) malloc(sizeof(uv_timer_t));
    uv_timer_init(m_loop, timer);

    // allocate a new waiter
    auto *waiter = new Waiter((uv_handle_t *) timer);
    waiter->promise = promise;

    // link the waiter to the promise and set the promise state to Pending
    promise->attach(waiter);

    // link the waiter to the timer handle
    timer->data = waiter;

    // start the timer
    uv_timer_start(timer, on_timer_complete, deadline, 0);

    // attach waiter to the end of the global waiters list
    if (m_waiters == nullptr) {
        waiter->prev = nullptr;
        waiter->next = nullptr;
        m_waiters = waiter;
    } else {
        auto *prev = m_waiters->prev;
        auto *next = m_waiters->next;
        if (prev == nullptr && next == nullptr) {
            m_waiters->prev = waiter;
            m_waiters->next = waiter;
            waiter->prev = m_waiters;
            waiter->next = m_waiters;
        } else {
            TU_ASSERT (prev != nullptr);
            TU_ASSERT (next != nullptr);
            waiter->prev = prev;
            waiter->next = m_waiters;
            prev->next = waiter;
            m_waiters->prev = waiter;
        }
    }
}

static void
on_async_complete(uv_async_t *async)
{
    auto *waiter = (lyric_runtime::Waiter *) async->data;

    // if there is a task associated with the waiter, then resume the suspended task.
    // we do this before running the waiter callback so that there will be a coroutine
    // available to the callback.
    if (waiter->task) {
        waiter->task->resume();
    }

    // if the waiter has an attached promise, then run the accept callback
    if (waiter->promise) {
        auto promise = waiter->promise;
        promise->accept();

        // if the promise has an adapt callback, then schedule it to be called before the task resumes
        if (promise->needsAdapt()) {
            auto *task = waiter->task;
            task->appendPromise(promise);
        }
    }

    // free the completed waiter
    auto *scheduler = waiter->task->getSystemScheduler();
    scheduler->destroyWaiter(waiter);
}

void
lyric_runtime::SystemScheduler::registerAsync(
    uv_async_t **asyncptr,
    std::shared_ptr<Promise> promise,
    tu_uint64 deadline)
{
    TU_ASSERT (asyncptr != nullptr);
    TU_ASSERT (promise != nullptr);
    TU_ASSERT (promise->getPromiseState() == PromiseState::Initial);

    // initialize the async handle
    auto *async = (uv_async_t *) malloc(sizeof(uv_async_t));
    uv_async_init(m_loop, async, on_async_complete);

    // allocate a new waiter
    auto *waiter = new Waiter((uv_handle_t *) async);
    waiter->promise = promise;

    // link the waiter to the promise and set the promise state to Pending
    promise->attach(waiter);

    // link the waiter to the async handle
    async->data = waiter;

    // attach waiter to the end of the global waiters list
    if (m_waiters == nullptr) {
        waiter->prev = nullptr;
        waiter->next = nullptr;
        m_waiters = waiter;
    } else {
        auto *prev = m_waiters->prev;
        auto *next = m_waiters->next;
        if (prev == nullptr && next == nullptr) {
            m_waiters->prev = waiter;
            m_waiters->next = waiter;
            waiter->prev = m_waiters;
            waiter->next = m_waiters;
        } else {
            TU_ASSERT (prev != nullptr);
            TU_ASSERT (next != nullptr);
            waiter->prev = prev;
            waiter->next = m_waiters;
            prev->next = waiter;
            m_waiters->prev = waiter;
        }
    }

    *asyncptr = async;
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

    auto *prev = waiter->prev;
    auto *next = waiter->next;

    // detach task from the wait queue
    if (prev != nullptr && next != nullptr) {
        if (prev == next) {
            next->next = nullptr;
            next->prev = nullptr;
        } else {
            prev->next = waiter->next;
            next->prev = waiter->prev;
        }
        if (waiter == m_waiters) {
            m_waiters = next;
        }
    } else {
        TU_ASSERT (prev == nullptr);
        TU_ASSERT (next == nullptr);
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