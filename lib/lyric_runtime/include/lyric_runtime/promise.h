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
    struct Waiter;

    /** Callback invoked to accept the event data from a waiter. */
    typedef void (*AcceptCallback)(Promise *, const Waiter *, InterpreterState *);

    /** Callback invoked to resolve the result of the future. */
    typedef void (*AdaptCallback)(Promise *, BytecodeInterpreter *, InterpreterState *);

    /** Callback invoked to release the void *data member when the Promise is deallocated */
    typedef void (*ReleaseCallback)(void *);

    /** Callback invoked to set reachable during GC any refs held in the void *data member */
    typedef void (*ReachableCallback)(void *);

    /**
     *
     */
    struct PromiseOptions {
        AdaptCallback adapt = nullptr;
        ReleaseCallback release = nullptr;
        ReachableCallback reachable = nullptr;
        void *data = nullptr;
    };

    /**
     *
     */
    class Promise final {
    public:
        static std::shared_ptr<Promise> create(AcceptCallback accept = {}, const PromiseOptions &options = {});
        static std::shared_ptr<Promise> completed(const DataCell &result);
        static std::shared_ptr<Promise> rejected(const DataCell &result);
        ~Promise();

        enum class State {
            Initial,    /**< The initial state of a promise before it is attached to a waiter. */
            Pending,    /**< The promise is pending a result. */
            Completed,  /**< The promise was completed. */
            Rejected,   /**< The promise was rejected. */
        };

        State getState() const;
        DataCell getResult() const;
        void *getData() const;

        void attach(Waiter *waiter);
        void await(SystemScheduler *systemScheduler);
        void accept(const Waiter *waiter, InterpreterState *state);
        void adapt(BytecodeInterpreter *interp, InterpreterState *state);
        bool needsAdapt() const;
        void setReachable();
        void complete(const DataCell &result);
        void reject(const DataCell &result);

    private:
        AcceptCallback m_accept;
        PromiseOptions m_options;

        State m_state;
        Waiter *m_waiter;
        DataCell m_result;  // FIXME: ensure result is reachable during GC

        Promise(State state, const DataCell &result);
        Promise(AcceptCallback accept, const PromiseOptions &options);
    };
}

#endif // LYRIC_RUNTIME_PROMISE_H
