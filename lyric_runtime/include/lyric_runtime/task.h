#ifndef LYRIC_RUNTIME_TASK_H
#define LYRIC_RUNTIME_TASK_H

#include <queue>

#include <uv.h>

#include "stackful_coroutine.h"

namespace lyric_runtime {

    // forward declarations
    class Promise;
    class SystemScheduler;

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

    /**
     *
     */
    class Task final {
    public:
        Task(TaskType type, SystemScheduler *scheduler);
        Task(TaskType type, SystemScheduler *scheduler, uv_async_t *worker);

        TaskType getTaskType() const;
        SystemScheduler *getSystemScheduler() const;
        StackfulCoroutine *stackfulCoroutine();

        TaskState getTaskState() const;
        void setTaskState(TaskState state);

        Task *prevTask() const;
        Task *nextTask() const;
        void attach(Task **head);
        void detach(Task **head);

        void suspend();
        void resume();
        void appendPromise(std::shared_ptr<Promise> promise);
        void adaptPromises(BytecodeInterpreter *interp, InterpreterState *state);

    private:
        const TaskType m_type;
        TaskState m_state;
        StackfulCoroutine m_coro;
        std::queue<std::shared_ptr<Promise>> m_promises;
        SystemScheduler *m_scheduler;
        uv_async_t *m_worker;
        Task *m_prev;
        Task *m_next;
    };
}

#endif //LYRIC_RUNTIME_TASK_H
