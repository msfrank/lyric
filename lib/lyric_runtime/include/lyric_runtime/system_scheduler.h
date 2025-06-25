#ifndef LYRIC_RUNTIME_SYSTEM_SCHEDULER_H
#define LYRIC_RUNTIME_SYSTEM_SCHEDULER_H

#include <uv.h>

#include "interpreter_result.h"
#include "promise.h"
#include "stackful_coroutine.h"
#include "task.h"

namespace lyric_runtime {

    class SystemScheduler;
    struct Waiter;

    /**
     *
     */
    struct Waiter final {
    private:
        Waiter *prev = nullptr;
        Waiter *next = nullptr;
        friend class SystemScheduler;
    public:
        uv_handle_t * const handle;
        Task *task = nullptr;
        std::shared_ptr<Promise> promise;
        explicit Waiter(uv_handle_t *handle): handle(handle) {};
        Waiter *prevWaiter() const { return prev; };
        Waiter *nextWaiter() const { return next; };
    };

    /**
     * The SystemScheduler manages and schedules tasks run by the interpreter, and is responsible for
     * polling the uv main loop.
     */
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
        void terminateTask(Task *task);
        void destroyTask(Task *task);

        void registerWorker(Task *workerTask, std::shared_ptr<Promise> promise);
        void registerTimer(tu_uint64 deadline, std::shared_ptr<Promise> promise);
        void registerAsync(uv_async_t **asyncptr, std::shared_ptr<Promise> promise, tu_uint64 deadline = 0);
        void destroyWaiter(Waiter *waiter);

        Waiter *firstWaiter() const;
        Waiter *lastWaiter() const;

        bool poll();
        bool blockingPoll();

    private:
        uv_loop_t *m_loop;
        Task *m_readyQueue;
        Task *m_waitQueue;
        Task *m_doneQueue;
        Task *m_currentTask;
        Task *m_mainTask;
        Waiter *m_waiters;

        void attachWaiter(Waiter *waiter);
    };
}

#endif // LYRIC_RUNTIME_SYSTEM_SCHEDULER_H