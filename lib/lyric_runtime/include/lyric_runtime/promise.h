#ifndef LYRIC_RUNTIME_PROMISE_H
#define LYRIC_RUNTIME_PROMISE_H

#include <memory>

#include "data_cell.h"

namespace lyric_runtime {

    // forward declarations
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
    class Promise final {
    public:
        explicit Promise(std::unique_ptr<PromiseOperations> ops);
        ~Promise();

        static std::shared_ptr<Promise> create(AcceptCallback accept = {});
        static std::shared_ptr<Promise> create(std::unique_ptr<PromiseOperations> ops);
        static std::shared_ptr<Promise> completed(const DataCell &result);
        static std::shared_ptr<Promise> rejected(const DataCell &result);

        enum class State {
            Initial,    /**< The initial state of a promise before it is attached to a waiter. */
            Pending,    /**< The promise is pending a result. */
            Completed,  /**< The promise was completed. */
            Rejected,   /**< The promise was rejected. */
        };

        State getState() const;
        DataCell getResult() const;

        void attach(Waiter *waiter);
        void await(SystemScheduler *systemScheduler);
        void accept(const Waiter *waiter, InterpreterState *state);
        void adapt(BytecodeInterpreter *interp, InterpreterState *state);
        void setReachable();

        void complete(const DataCell &result);
        void reject(const DataCell &result);

    private:
        std::unique_ptr<PromiseOperations> m_ops;

        State m_state;
        Waiter *m_waiter;
        DataCell m_result;
    };
}

#endif // LYRIC_RUNTIME_PROMISE_H
