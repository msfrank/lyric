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
     *
     */
    class Task final {
    public:
        Task(bool isMainTask, SystemScheduler *scheduler);

        SystemScheduler *getSystemScheduler() const;
        StackfulCoroutine *stackfulCoroutine();

        bool isMainTask() const;

        enum class State {
            Initial,    /**< The initial state of a task before it is added to the scheduler. */
            Waiting,    /**< The task is in the waiting queue. */
            Ready,      /**< The task is in the ready queue. */
            Running,    /**< The task is currently running. */
            Done,       /**< The task has finished and is in the done queue. */
        };

        State getState() const;
        void setState(State state);

        uv_async_t *getMonitor() const;
        void setMonitor(uv_async_t *monitor);
        void signalMonitor();

        Task *prevTask() const;
        Task *nextTask() const;
        void attach(Task **head);
        void detach(Task **head);

        void suspend();
        void resume();
        void appendPromise(std::shared_ptr<Promise> promise);
        void adaptPromises(BytecodeInterpreter *interp, InterpreterState *state);

    private:
        bool m_isMainTask;
        State m_state;
        StackfulCoroutine m_coro;
        std::queue<std::shared_ptr<Promise>> m_promises;
        SystemScheduler *m_scheduler;
        uv_async_t *m_monitor;
        Task *m_prev;
        Task *m_next;
    };
}

#endif //LYRIC_RUNTIME_TASK_H
