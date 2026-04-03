#ifndef LYRIC_RUNTIME_SYSTEM_SCHEDULER_H
#define LYRIC_RUNTIME_SYSTEM_SCHEDULER_H

#include <uv.h>

#include "promise.h"
#include "stackful_coroutine.h"
#include "task.h"

namespace lyric_runtime {

    class SystemScheduler;
    class Waiter;

    /**
     *
     */
    class AsyncHandle {
    public:
        AsyncHandle(uv_async_t *async, uv_close_cb cb);
        ~AsyncHandle();

        bool isPending() const;
        bool sendSignal();

    private:
        std::unique_ptr<absl::Mutex> m_lock;
        uv_async_t *m_async ABSL_GUARDED_BY(*m_lock);
        uv_close_cb m_cb ABSL_GUARDED_BY(*m_lock);
        bool m_pending ABSL_GUARDED_BY(*m_lock);

        void close();

        friend class SystemScheduler;
        friend class Waiter;
    };

    /**
     *
     */
    class Waiter final {
    public:
        ~Waiter();

        enum class Type {
            Invalid,
            Async,
            Handle,
            Req,
        };

        Type getType() const;
        bool hasTask() const;
        bool hasPromise() const;

        const AsyncHandle *peekAsync() const;
        const uv_handle_t *peekHandle() const;
        const uv_fs_t *peekReq() const;

        Waiter *prevWaiter() const;
        Waiter *nextWaiter() const;

    private:
        Waiter *m_prev = nullptr;
        Waiter *m_next = nullptr;

        Type m_type;
        std::variant<std::nullptr_t, std::shared_ptr<AsyncHandle>, uv_handle_t *, uv_fs_t *> m_waitee;
        uv_timer_t *m_deadline = nullptr;
        Task *m_task = nullptr;
        std::shared_ptr<Promise> m_promise;

        explicit Waiter(std::shared_ptr<AsyncHandle> async, std::shared_ptr<Promise> promise = {});
        explicit Waiter(uv_handle_t *handle, std::shared_ptr<Promise> promise = {});
        explicit Waiter(uv_fs_t *req, std::shared_ptr<Promise> promise = {});

        Task *getTask() const;
        void assignTask(Task *task);
        std::shared_ptr<Promise> getPromise() const;

        friend class SystemScheduler;
        friend class HeapManager;

        friend void on_async_complete(uv_async_t *async);
        friend void on_timer_complete(uv_timer_t *timer);
        friend void on_worker_complete(uv_async_t *monitor);
        friend void on_read_complete(uv_fs_t *req);
        friend void on_write_complete(uv_fs_t *req);
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

        void await(Waiter *waiter);

        tempo_utils::Result<std::shared_ptr<AsyncHandle>> registerAsync(
            std::shared_ptr<Promise> promise,
            tu_uint64 deadline = 0);

        tempo_utils::Status registerWorker(Task *workerTask, std::shared_ptr<Promise> promise);
        tempo_utils::Status registerTimer(tu_uint64 deadline, std::shared_ptr<Promise> promise);
        tempo_utils::Status registerRead(
            uv_file file,
            uv_buf_t buf,
            std::shared_ptr<Promise> promise,
            tu_int64 offset = -1);
        tempo_utils::Status registerWrite(
            uv_file file,
            uv_buf_t buf,
            std::shared_ptr<Promise> promise,
            tu_int64 offset = -1);

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