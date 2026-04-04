
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/system_scheduler.h>

lyric_runtime::AsyncHandle::AsyncHandle(uv_async_t *async, uv_close_cb cb)
    : m_async(async),
      m_cb(cb),
      m_pending(true)
{
    m_lock = std::make_unique<absl::Mutex>();
    TU_NOTNULL (m_async);
}

lyric_runtime::AsyncHandle::~AsyncHandle()
{
    close();
}

/**
 * Indicates whether the AsyncHandle is in pending state.
 *
 * @return true if the AsyncHandle is pending, otherwise false.
 */
bool
lyric_runtime::AsyncHandle::isPending() const
{
    absl::MutexLock locker(m_lock.get());
    return m_pending;
}

/**
 * If AsyncHandle is pending then trigger the async callback, otherwise do nothing.
 *
 * @return true if the signal was sent, otherwise false.
 */
bool
lyric_runtime::AsyncHandle::sendSignal()
{
    absl::MutexLock locker(m_lock.get());
    if (m_async && m_pending) {
        uv_async_send(m_async);
        m_pending = false;
        return true;
    }
    return false;
}

/**
 * Closes the internal async handle and marks it for deletion. After this method has been called
 * `sendSignal` will do nothing, and `isPending` will return false.
 */
void
lyric_runtime::AsyncHandle::close()
{
    absl::MutexLock locker(m_lock.get());
    if (m_async) {
        uv_close((uv_handle_t *) m_async, m_cb);
        m_async = nullptr;
        m_pending = false;
    }
}

lyric_runtime::Waiter::Waiter(std::shared_ptr<AsyncHandle> async, std::shared_ptr<Promise> promise)
    : m_type(Type::Async),
      m_promise(std::move(promise))
{
    TU_NOTNULL (m_promise);
    TU_NOTNULL (async);
    m_waitee = std::move(async);
}

lyric_runtime::Waiter::Waiter(uv_handle_t *handle, std::shared_ptr<Promise> promise)
    : m_type(Type::Handle),
      m_promise(std::move(promise))
{
    TU_NOTNULL (m_promise);
    TU_NOTNULL (handle);
    m_waitee = handle;
}

lyric_runtime::Waiter::Waiter(uv_fs_t *req, std::shared_ptr<Promise> promise)
    : m_type(Type::Req),
      m_promise(std::move(promise))
{
    TU_NOTNULL (m_promise);
    TU_NOTNULL (req);
    m_waitee = req;
}

static void
on_handle_close(uv_handle_t *handle)
{
    free(handle);
}

lyric_runtime::Waiter::~Waiter()
{
    switch (m_type) {
        case Type::Async: {
            auto async = std::get<std::shared_ptr<AsyncHandle>>(m_waitee);
            // close async handle if still pending, otherwise this is a no-op
            async->close();
            break;
        }
        case Type::Handle: {
            auto *handle = std::get<uv_handle_t *>(m_waitee);
            // close the handle and defer deallocation to the specified callback
            uv_close(handle, on_handle_close);
            break;
        }
        case Type::Req: {
            auto *req = std::get<uv_fs_t *>(m_waitee);
            // clean up the fs request immediately
            uv_fs_req_cleanup(req);
            break;
        }
        default:
            TU_UNREACHABLE();
    }
}

lyric_runtime::Waiter::Type
lyric_runtime::Waiter::getType() const
{
    return m_type;
}

const lyric_runtime::AsyncHandle *
lyric_runtime::Waiter::peekAsync() const
{
    if (static_cast<Type>(m_waitee.index()) != Type::Async)
        return nullptr;
    auto async = std::get<std::shared_ptr<AsyncHandle>>(m_waitee);
    return async.get();
}

const uv_handle_t *
lyric_runtime::Waiter::peekHandle() const
{
    if (static_cast<Type>(m_waitee.index()) != Type::Handle)
        return nullptr;
    return std::get<uv_handle_t *>(m_waitee);
}

const uv_fs_t *
lyric_runtime::Waiter::peekReq() const
{
    if (static_cast<Type>(m_waitee.index()) != Type::Req)
        return nullptr;
    return std::get<uv_fs_t *>(m_waitee);
}

bool
lyric_runtime::Waiter::hasTask() const
{
    return m_task != nullptr;
}

bool
lyric_runtime::Waiter::hasPromise() const
{
    return m_promise != nullptr;
}

lyric_runtime::Waiter *
lyric_runtime::Waiter::prevWaiter() const
{
    return m_prev;
}

lyric_runtime::Waiter *
lyric_runtime::Waiter::nextWaiter() const
{
    return m_next;
}

/**
 * If a promise is assigned to the waiter then set its refs reachable during GC.
 */
void
lyric_runtime::Waiter::setReachable()
{
    if (m_promise) {
        m_promise->setReachable();
    }
}

/**
 * Assign the specified task to the waiter. It is a precondition that the waiter not have a task already
 * assigned.
 *
 * @param task The task.
 */
void
lyric_runtime::Waiter::assignTask(Task *task)
{
    TU_NOTNULL (task);
    TU_ASSERT (m_task == nullptr);
    m_task = task;
}

/**
 * Complete the waiter and release waiter resources. After calling this method the Waiter is destroyed
 * and all pointers to the Waiter must be discarded.
 *
 * @param state The interpreter state.
 */
void
lyric_runtime::Waiter::complete(InterpreterState *state)
{
    TU_NOTNULL (state);

    // if there is a task associated with the waiter, then resume the suspended task.
    // we do this before running the waiter callback so that there will be a coroutine
    // available to the callback. m_task can be nullptr if the waiter is a dependency
    // of another waiter.
    if (m_task != nullptr) {
        m_task->resume();
    }

    // if the waiter has an attached promise, then run the accept callback
    if (m_promise != nullptr) {
        m_promise->accept(this, state);

        // // if the promise has an adapt callback, then schedule it to be called before the task resumes
        // if (promise->needsAdapt()) {
        //     auto *task = waiter->getTask();
        //     TU_NOTNULL (task);
        //     task->appendPromise(promise);
        // }
    }

    // free the completed waiter
    auto *systemScheduler = state->systemScheduler();
    systemScheduler->destroyWaiter(this);
}

lyric_runtime::SystemScheduler::SystemScheduler(uv_loop_t *loop)
    : m_loop(loop),
      m_readyQueue(nullptr),
      m_waitQueue(nullptr),
      m_doneQueue(nullptr),
      m_currentTask(nullptr),
      m_mainTask(nullptr),
      m_waiters(nullptr)
{
    TU_ASSERT (m_loop != nullptr);

    m_mainTask = new Task(/* isMainTask= */ true, this);
    m_mainTask->setState(Task::State::Waiting);

    // attach main task to the end of the wait queue
    m_mainTask->attach(&m_waitQueue);
}

lyric_runtime::SystemScheduler::~SystemScheduler()
{
    // move ready tasks to the done queue
    while (m_readyQueue) {
        terminateTask(m_readyQueue);
    }
    // move waiting tasks to the done queue
    while (m_waitQueue) {
        terminateTask(m_waitQueue);
    }
    // destroy all terminated tasks
    while (m_doneQueue) {
        destroyTask(m_doneQueue);
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

/**
 * Select the next ready task an return a pointer to the task. If there is no ready task available
 * then return nullptr.
 *
 * @return A pointer to the next ready task, or nullptr if there is no ready task available.
 */
lyric_runtime::Task *
lyric_runtime::SystemScheduler::selectNextReady()
{
    // if there is no current task, then select the top of the ready queue
    if (m_currentTask == nullptr) {
        m_currentTask = m_readyQueue;
        if (m_currentTask) {
            // if ready queue has an available task then set it to running state
            m_currentTask->setState(Task::State::Running);
        }
        // return the current task, which may be null!
        return m_currentTask;
    }

    // if there is a current task but no other ready tasks, then keep the current task running
    if (m_currentTask->nextTask() == nullptr)
        return m_currentTask;

    // otherwise switch current task state to ready
    m_currentTask->setState(Task::State::Ready);

    // then select the next task
    auto *selected = m_currentTask->nextTask();
    selected->setState(Task::State::Running);
    m_currentTask = selected;
    return m_currentTask;
}

lyric_runtime::Task *
lyric_runtime::SystemScheduler::createTask()
{
    // create new worker task and attach it to the end of the wait queue
    auto *task = new Task(/* isMainTask= */ false, this);
    task->setState(Task::State::Waiting);
    task->attach(&m_waitQueue);
    return task;
}

void
lyric_runtime::SystemScheduler::suspendTask(Task *task)
{
    TU_ASSERT (task != nullptr);

    switch (task->getState()) {
        case Task::State::Waiting:
            return;     // if task is already suspended, then do nothing
        case Task::State::Ready:
        case Task::State::Running:
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
    task->setState(Task::State::Waiting);

    // attach task to the end of the wait queue
    task->attach(&m_waitQueue);
}

void
lyric_runtime::SystemScheduler::resumeTask(Task *task)
{
    TU_ASSERT (task != nullptr);

    switch (task->getState()) {
        case Task::State::Ready:
        case Task::State::Running:
            return;     // if task is ready or running, then do nothing
        case Task::State::Waiting:
            break;
        default:
            TU_LOG_FATAL << "task " << task << " cannot be resumed; unexpected state";
            return;
    }

    // detach task from the wait queue
    task->detach(&m_waitQueue);

    // set the task state to Ready
    task->setState(Task::State::Ready);

    // attach task to the end of the ready queue
    task->attach(&m_readyQueue);
}

void
lyric_runtime::SystemScheduler::terminateTask(Task *task)
{
    TU_ASSERT (task != nullptr);

    switch (task->getState()) {
        case Task::State::Initial:
            return;
        case Task::State::Running:
            m_currentTask = nullptr;
            task->detach(&m_readyQueue);
            break;
        case Task::State::Ready:
            task->detach(&m_readyQueue);
            break;
        case Task::State::Waiting:
            task->detach(&m_waitQueue);
            break;
        default:
            TU_LOG_FATAL << "task " << task << " cannot be terminated; unexpected state";
            return;
    }

    // set the task state to Done
    task->setState(Task::State::Done);

    // attach task to the end of the done queue
    task->attach(&m_doneQueue);

    // signal the monitor that the task was terminated
    task->signalMonitor();
}

void
lyric_runtime::SystemScheduler::destroyTask(Task *task)
{
    TU_ASSERT (task != nullptr);
    TU_ASSERT (task->getState() == Task::State::Done);

    // detach task from the done queue and deallocate it
    task->detach(&m_doneQueue);
    delete task;
}

void
lyric_runtime::SystemScheduler::await(Waiter *waiter)
{
    TU_NOTNULL (waiter);
    auto *currentTask = m_currentTask;

    // associate waiter with the current task
    waiter->assignTask(currentTask);

    // suspend the current task and set state to Waiting
    suspendTask(currentTask);
}

void
lyric_runtime::on_async_complete(uv_async_t *async)
{
    auto *waiter = static_cast<Waiter *>(async->data);
    auto *state = static_cast<InterpreterState *>(async->loop->data);
    waiter->complete(state);
}

static void
on_async_close(uv_handle_t *async)
{
    std::free(async);
}

tempo_utils::Result<std::shared_ptr<lyric_runtime::AsyncHandle>>
lyric_runtime::SystemScheduler::registerAsync(
    std::shared_ptr<Promise> promise,
    tu_uint64 deadline)
{
    TU_ASSERT (promise != nullptr);
    TU_ASSERT (promise->getState() == Promise::State::Initial);

    // initialize the async handle
    auto *handle = (uv_async_t *) malloc(sizeof(uv_async_t));
    uv_async_init(m_loop, handle, on_async_complete);
    auto async = std::make_shared<AsyncHandle>(handle, on_async_close);

    // allocate a new waiter
    auto *waiter = new Waiter(async, promise);

    // link the waiter to the promise and set the promise state to Pending
    promise->attach(waiter);

    // link the waiter to the async handle
    handle->data = waiter;

    // attach waiter to the end of the global waiters list
    attachWaiter(waiter);

    // return async handle
    return async;
}

void
lyric_runtime::on_worker_complete(uv_async_t *monitor)
{
    auto *waiter = static_cast<Waiter *>(monitor->data);
    auto *state = static_cast<InterpreterState *>(monitor->loop->data);
    waiter->complete(state);
}

tempo_utils::Status
lyric_runtime::SystemScheduler::registerWorker(Task *workerTask, std::shared_ptr<Promise> promise)
{
    TU_ASSERT (promise != nullptr);
    TU_ASSERT (promise->getState() == Promise::State::Initial);

    // initialize the async handle
    auto *monitor = (uv_async_t *) malloc(sizeof(uv_async_t));
    uv_async_init(m_loop, monitor, on_worker_complete);

    // allocate a new waiter
    auto *waiter = new Waiter((uv_handle_t *) monitor, promise);

    // link the waiter to the promise and set the promise state to Pending
    promise->attach(waiter);

    // link the waiter to the async handle
    monitor->data = waiter;

    // attach waiter to the end of the global waiters list
    attachWaiter(waiter);

    //
    workerTask->setMonitor(monitor);

    return {};
}

void
lyric_runtime::on_timer_complete(uv_timer_t *timer)
{
    auto *waiter = static_cast<Waiter *>(timer->data);
    auto *state = static_cast<InterpreterState *>(timer->loop->data);
    waiter->complete(state);
}

tempo_utils::Status
lyric_runtime::SystemScheduler::registerTimer(tu_uint64 deadline, std::shared_ptr<Promise> promise)
{
    TU_ASSERT (promise != nullptr);
    TU_ASSERT (promise->getState() == Promise::State::Initial);

    // initialize the timer handle
    auto *timer = (uv_timer_t *) malloc(sizeof(uv_timer_t));
    uv_timer_init(m_loop, timer);

    // allocate a new waiter
    auto *waiter = new Waiter((uv_handle_t *) timer, promise);

    // link the waiter to the promise and set the promise state to Pending
    promise->attach(waiter);

    // link the waiter to the timer handle
    timer->data = waiter;

    // start the timer
    uv_timer_start(timer, on_timer_complete, deadline, 0);

    // attach waiter to the end of the global waiters list
    attachWaiter(waiter);

    return {};
}

void
lyric_runtime::on_read_complete(uv_fs_t *req)
{
    auto *waiter = static_cast<Waiter *>(req->data);
    auto *state = static_cast<InterpreterState *>(req->loop->data);
    waiter->complete(state);
}

tempo_utils::Status
lyric_runtime::SystemScheduler::registerRead(
    uv_file file,
    uv_buf_t buf,
    std::shared_ptr<Promise> promise,
    tu_int64 offset)
{
    TU_ASSERT (promise != nullptr);
    TU_ASSERT (promise->getState() == Promise::State::Initial);

    auto *req = (uv_fs_t *) malloc(sizeof(uv_fs_t));
    memset(req, 0, sizeof(uv_fs_t));

    auto ret = uv_fs_read(m_loop, req, file, &buf, 1, offset, on_read_complete);
    if (ret < 0) {
        uv_fs_req_cleanup(req);
        free(req);
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "failed to schedule read: {}", uv_strerror(ret));
    }

    // allocate a new waiter
    auto *waiter = new Waiter(req, promise);

    // link the waiter to the promise and set the promise state to Pending
    promise->attach(waiter);

    // link the waiter to the fs request
    req->data = waiter;

    // attach waiter to the end of the global waiters list
    attachWaiter(waiter);

    return {};
}

void
lyric_runtime::on_write_complete(uv_fs_t *req)
{
    auto *waiter = static_cast<Waiter *>(req->data);
    auto *state = static_cast<InterpreterState *>(req->loop->data);
    waiter->complete(state);
}

tempo_utils::Status
lyric_runtime::SystemScheduler::registerWrite(
    uv_file file,
    uv_buf_t buf,
    std::shared_ptr<Promise> promise,
    tu_int64 offset)
{
    TU_ASSERT (promise != nullptr);
    TU_ASSERT (promise->getState() == Promise::State::Initial);

    auto *req = (uv_fs_t *) malloc(sizeof(uv_fs_t));
    memset(req, 0, sizeof(uv_fs_t));

    auto ret = uv_fs_write(m_loop, req, file, &buf, 1, offset, on_write_complete);
    if (ret < 0) {
        uv_fs_req_cleanup(req);
        free(req);
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "failed to schedule write: {}", uv_strerror(ret));
    }

    // allocate a new waiter
    auto *waiter = new Waiter(req, promise);

    // link the waiter to the promise and set the promise state to Pending
    promise->attach(waiter);

    // link the waiter to the fs request
    req->data = waiter;

    // attach waiter to the end of the global waiters list
    attachWaiter(waiter);

    return {};
}

void
lyric_runtime::SystemScheduler::attachWaiter(Waiter *waiter)
{
    TU_NOTNULL (waiter);
    TU_ASSERT (waiter->m_prev == nullptr);
    TU_ASSERT (waiter->m_next == nullptr);

    if (m_waiters == nullptr) {
        waiter->m_prev = nullptr;
        waiter->m_next = nullptr;
        m_waiters = waiter;
    } else {
        auto *prev = m_waiters->m_prev;
        auto *next = m_waiters->m_next;
        if (prev == nullptr && next == nullptr) {
            m_waiters->m_prev = waiter;
            m_waiters->m_next = waiter;
            waiter->m_prev = m_waiters;
            waiter->m_next = m_waiters;
        } else {
            TU_NOTNULL (prev);
            TU_NOTNULL (next);
            waiter->m_prev = prev;
            waiter->m_next = m_waiters;
            prev->m_next = waiter;
            m_waiters->m_prev = waiter;
        }
    }
}

void
lyric_runtime::SystemScheduler::destroyWaiter(Waiter *waiter)
{
    TU_NOTNULL (waiter);

    auto *prev = waiter->m_prev;
    auto *next = waiter->m_next;

    // detach task from the wait queue
    if (prev != nullptr && next != nullptr) {
        if (prev == next) {
            next->m_next = nullptr;
            next->m_prev = nullptr;
        } else {
            prev->m_next = waiter->m_next;
            next->m_prev = waiter->m_prev;
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

lyric_runtime::Waiter *
lyric_runtime::SystemScheduler::firstWaiter() const
{
    return m_waiters;
}

lyric_runtime::Waiter *
lyric_runtime::SystemScheduler::lastWaiter() const
{
    if (m_waiters == nullptr)
        return nullptr;
    return m_waiters->m_prev;
}

/**
 * Pump the event loop one time then return immediately without blocking.
 *
 * @return true if an event was processed, otherwise false.
 */
bool
lyric_runtime::SystemScheduler::poll()
{
    // TODO: return InterpreterStatus instead of bool
    auto result = uv_run(m_loop, UV_RUN_NOWAIT);
    return result == 0;
}

/**
 * If there are no ready tasks then block the interpreter waiting for an event, and return after the
 * event has been handled. If there is at least one ready task then return immediately without running
 * the event loop. Note that it is not guaranteed there will be an available ready task after this
 * call returns.
 *
 * @return true if an event was processed, otherwise false.
 */
bool
lyric_runtime::SystemScheduler::blockingPoll()
{
    // if there is a ready task then return immediately without blocking
    if (m_readyQueue != nullptr)
        return false;
    // if there are no ready tasks and no waiting tasks then throw exception
    if (m_waitQueue == nullptr)
        throw tempo_utils::StatusException(
            InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "aborting blocking poll: no tasks are waiting"));
    uv_run(m_loop, UV_RUN_ONCE);
    return true;
}