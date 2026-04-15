#ifndef LYRIC_RUNTIME_PROMISE_H
#define LYRIC_RUNTIME_PROMISE_H

#include <memory>

#include "data_cell.h"
#include "system_scheduler.h"

namespace lyric_runtime {

    // forward declarations
    class AsyncHandle;
    class BytecodeInterpreter;
    class Promise;
    class InterpreterState;
    class SystemScheduler;
    class Waiter;

    using AcceptCallback = std::function<void(Promise *, const Waiter *, InterpreterState *)>;

    /**
     *
     */
    class PromiseOperations {
    public:
        virtual ~PromiseOperations() = default;
        /**
         * Callback invoked to accept the event data from a waiter. The default implementation does nothing.
         */
        virtual void onAccept(Promise *promise, const Waiter *waiter, InterpreterState *state) {};
        /**
         * Callback invoked when Promise is a target and one or more of its dependencies have finished. If the
         * supplied AsyncHandle is signaled, then the Promise begins resolution. The default implementation
         * immediately signals the AsyncHandle.
         */
        virtual void onPartial(
            std::vector<std::shared_ptr<Promise>>::const_iterator depsBegin,
            std::vector<std::shared_ptr<Promise>>::const_iterator depsEnd,
            std::shared_ptr<AsyncHandle> &done);
        /**
         * Callback invoked to resolve the result of the future. The default implementation does nothing.
         */
        virtual void onAdapt(Promise *promise, BytecodeInterpreter *interp, InterpreterState *state) {};
        /**
         * Callback invoked to set refs reachable during GC. The default implementation does nothing.
         */
        virtual void setReachable() {};
    };

    /**
     * A Promise is a placeholder for a result that is placed on the top of the call stack when a task
     * has finished awaiting.
     */
    class Promise final : public std::enable_shared_from_this<Promise> {

        struct Private {};

    public:
        Promise(std::unique_ptr<PromiseOperations> ops, Private);
        ~Promise();

        static std::shared_ptr<Promise> create(AcceptCallback accept = {});
        static std::shared_ptr<Promise> create(std::unique_ptr<PromiseOperations> ops);
        static std::shared_ptr<Promise> completed(const DataCell &result);
        static std::shared_ptr<Promise> rejected(const DataCell &result);

        enum class State {
            Initial,    /**< The initial state of a promise before it is attached to a waiter. */
            Pending,    /**< The promise is pending a result. */
            Target,     /**< The promise is marked as a target. */
            Forwarded,  /**< The promise is forwarded to a target promise. */
            Waiting,    /**< The promise is being awaited. */
            Completed,  /**< The promise was completed. */
            Rejected,   /**< The promise was rejected. */
        };

        State getState() const;
        DataCell getResult() const;

        tempo_utils::Status pending(Waiter *waiter);
        tempo_utils::Status target(std::shared_ptr<AsyncHandle> async, Waiter *waiter);

        tempo_utils::Status await(SystemScheduler *systemScheduler);
        tempo_utils::Status forward(std::shared_ptr<Promise> &target);

        void accept(const Waiter *waiter, InterpreterState *state);
        void adapt(BytecodeInterpreter *interp, InterpreterState *state);
        void setReachable();

        tempo_utils::Status complete(const DataCell &result);
        tempo_utils::Status reject(const DataCell &result);

    private:
        std::unique_ptr<PromiseOperations> m_ops;
        std::vector<std::shared_ptr<Promise>> m_dependencies;
        std::weak_ptr<Promise> m_target;
        State m_state;
        Waiter *m_waiter;
        std::shared_ptr<AsyncHandle> m_async;
        DataCell m_result;

        tempo_utils::Status notify();
    };
}

#endif // LYRIC_RUNTIME_PROMISE_H
