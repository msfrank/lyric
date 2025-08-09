#ifndef LYRIC_BUILD_BUILD_RUNNER_H
#define LYRIC_BUILD_BUILD_RUNNER_H

#include <mutex>
#include <queue>
#include <random>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <uv.h>

#include <lyric_build/abstract_cache.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/task_notification.h>
#include <lyric_build/task_registry.h>
#include <tempo_tracing/trace_recorder.h>

namespace lyric_build {

    /**
     *
     */
    enum class RunnerState {
        Ready,
        Running,
        Shutdown,
        Done,
    };

    struct ReadyItem {
        enum class Type {
            TASK,
            SHUTDOWN,
            TIMEOUT,
        };
        Type type;
        BaseTask *task;
    };

    /**
     *
     */
    class AbstractBuildRunner {
    public:
        virtual ~AbstractBuildRunner() = default;

        virtual tempo_utils::Status enqueueTask(const TaskKey &key) = 0;

        virtual ReadyItem waitForNextReady(int timeout) = 0;

        virtual tempo_utils::Status enqueueNotification(std::unique_ptr<TaskNotification> notification) = 0;
    };

    struct TaskThread {
        AbstractBuildRunner *runner = nullptr;
        const TaskSettings *taskSettings = nullptr;
        std::shared_ptr<BuildState> buildState;
        std::shared_ptr<AbstractCache> buildCache;
        int index = -1;
        uv_thread_t tid;
        bool running = false;
        bool joined = false;
    };

    /**
     *
     */
    class BuildRunner : public AbstractBuildRunner {

    public:
        BuildRunner(
            const TaskSettings *taskSettings,
            std::shared_ptr<BuildState> buildState,
            std::shared_ptr<AbstractCache> buildCache,
            TaskRegistry *taskRegistry,
            int numThreads,
            int waitTimeoutInMs,
            TaskNotificationFunc onNotificationFunc,
            void *onNotificationData);
        ~BuildRunner() override;

        const TaskSettings *getConfig() const;
        std::shared_ptr<BuildState> getState() const;
        std::shared_ptr<AbstractCache> getCache() const;
        TaskRegistry *getRegistry() const;

        tempo_utils::Status enqueueTask(const TaskKey &key) override;
        ReadyItem waitForNextReady(int timeout) override;

        tempo_utils::Status enqueueNotification(std::unique_ptr<TaskNotification> notification) override;
        std::unique_ptr<std::queue<std::unique_ptr<TaskNotification>>> takeNotifications();

        tempo_utils::Status run();

        std::shared_ptr<tempo_tracing::TraceSpan> makeSpan();

        tempo_utils::Status parkDeps(const TaskKey &key, const absl::flat_hash_set<TaskKey> &dependencies);
        tempo_utils::Status restartDeps(const TaskKey &key);
        absl::flat_hash_set<TaskKey> getWaiting(const TaskKey &key);
        absl::flat_hash_set<TaskKey> getBlocked(const TaskKey &key);

        void joinThread(int index);
        void invokeNotificationCallback(std::unique_ptr<TaskNotification> notification);

        tempo_utils::Result<tempo_tracing::TempoSpanset> getSpanset() const;
        int getTotalTasksCreated() const;
        int getTotalTasksCached() const;

        tempo_utils::Status shutdown(const tempo_utils::Status &shutdownStatus = {});

    private:

        const TaskSettings *m_config;
        std::shared_ptr<BuildState> m_state;
        std::shared_ptr<AbstractCache> m_cache;
        TaskRegistry *m_registry;

        // build diagnostics recorder
        std::shared_ptr<tempo_tracing::TraceRecorder> m_recorder;

        // members below are set after runner is finished

        int m_totalTasksCreated;
        int m_totalTasksCached;

        // members below are read only

        const int m_numThreads;
        const int m_waitTimeoutInMs;                        // wait condition polling timeout

        // uv data
        uv_loop_t m_loop;                                   // uv main loop handle
        uv_async_t m_asyncNotify;                           // async handle to process notifications in the main loop

        std::mutex m_threadsLock;                           // lock around threads
        std::vector<TaskThread> m_threads;                  // array containing thread data

        // members below can be accessed from multiple threads after acquiring the appropriate lock

        std::mutex m_statusLock;                            // lock around runnerState and shutdownStatus
        RunnerState m_runnerState;                          // runner state
        tempo_utils::Status m_shutdownStatus;               // shutdown status

        std::shared_mutex m_tasksRWlock;                    // readwrite lock around the tasks map
        absl::flat_hash_map<TaskKey, BaseTask *> m_tasks;   // map of build tasks

        std::timed_mutex m_readyLock;                       // lock around the ready queue
        std::queue<ReadyItem> m_ready;                      // queue of ready items in FIFO order
        absl::flat_hash_set<TaskKey> m_queued;              // set of ready tasks which are in the queue

        std::mutex m_waitLock;                              // lock around the readyWaiter
        std::condition_variable m_readyWaiter;              // condition variable which signals when there is a ready task

        std::timed_mutex m_notificationLock;                // lock around the notifications queue
        std::unique_ptr<
            std::queue<
                std::unique_ptr<TaskNotification>>>
        m_notifications;                                    // queue of notifications in FIFO order

        std::mutex m_randLock;                              // lock around the random engine
        std::mt19937 m_randengine;                          // random number generator engine

        // members below are only accessed by the monitor thread

        absl::flat_hash_map<
            TaskKey,
            absl::flat_hash_set<TaskKey>>
            m_blocked;                                      // key is blocked task, value is set of dependencies
        absl::flat_hash_map<
            TaskKey,
            absl::flat_hash_set<TaskKey>>
            m_waiting;                                      // key is dependency, value is set of waiting tasks
        TaskNotificationFunc m_onNotificationFunc;          // called in main loop when a notification is received
        void *m_onNotificationData;                         // data pointer passed to onNotificationFunc
    };
}

#endif // LYRIC_BUILD_BUILD_RUNNER_H