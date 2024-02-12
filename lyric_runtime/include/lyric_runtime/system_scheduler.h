#ifndef LYRIC_RUNTIME_SYSTEM_SCHEDULER_H
#define LYRIC_RUNTIME_SYSTEM_SCHEDULER_H

#include <uv.h>

#include "interpreter_result.h"
#include "stackful_coroutine.h"

namespace lyric_runtime {

    class SystemScheduler;
    struct Waiter;
    typedef void (*WaiterCallback)(Waiter *, void *);

    /**
     * The task type.
     */
    enum class TaskType {
        Main,       /**< Main task. There can only be a single main task for a running interpreter. */
        Worker,     /**< Worker task. There can be many concurrent worker tasks in a running interpreter. */
    };

    /**
     * The task state.
     */
    enum class TaskState {
        Initial,    /**< The initial state of a task before it is added to the scheduler. */
        Waiting,    /**< The task is in the waiting queue. */
        Ready,      /**< The task is in the ready queue. */
        Running,    /**< The task is currently running. */
    };

    struct Task {
        TaskType type;
        TaskState state = TaskState::Initial;
        StackfulCoroutine coro;
        SystemScheduler *scheduler = nullptr;
        uv_async_t *worker = nullptr;
        Task *prev = nullptr;
        Task *next = nullptr;
        explicit Task(TaskType type): type(type) {};
    };

    struct Waiter {
        Task *task = nullptr;
        uv_handle_t *handle = nullptr;
        WaiterCallback cb = nullptr;
        void *data = nullptr;
        Waiter *prev = nullptr;
        Waiter *next = nullptr;
        explicit Waiter(Task *task): task(task) {};
    };

    class SystemScheduler {
    public:
        explicit SystemScheduler(uv_loop_t *loop);
        ~SystemScheduler();

        uv_loop_t *systemLoop() const;

        Task *mainTask() const;
        StackfulCoroutine *mainCoro() const;
        Task *currentTask() const;
        StackfulCoroutine *currentCoro() const;

        Task *firstReadyTask() const;
        Task *firstWaitingTask() const;

        Task *selectNextReady();
        Task *createTask();
        void suspendTask(Task *task);
        void resumeTask(Task *task);
        void destroyTask(Task *task);

        Waiter *registerTimer(uint64_t timeout, WaiterCallback cb, void *data);
        Waiter *registerAsync(uv_async_t **asyncptr, WaiterCallback cb, void *data);
        void destroyWaiter(Waiter *waiter);

        bool poll();
        bool blockingPoll();

    private:
        uv_loop_t *m_loop;
        Task *m_readyQueue;
        Task *m_waitQueue;
        Task *m_currentTask;
        Task *m_mainTask;
        Waiter *m_waiters;
    };
}

#endif // LYRIC_RUNTIME_SYSTEM_SCHEDULER_H