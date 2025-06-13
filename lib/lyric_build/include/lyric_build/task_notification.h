#ifndef LYRIC_BUILD_TASK_NOTIFICATION_H
#define LYRIC_BUILD_TASK_NOTIFICATION_H

#include <absl/container/flat_hash_set.h>

#include "build_types.h"

namespace lyric_build {

    enum class NotificationType {
        INVALID,
        STATE_CHANGED,
        TASK_REQUESTED,
        TASK_BLOCKED,
        TASK_UNBLOCKED,
        THREAD_CANCELLED,
    };

    class TaskNotification {

    public:
        TaskNotification();
        virtual ~TaskNotification() = default;
        NotificationType getType() const;
        virtual std::string toString() const = 0;

    protected:
        explicit TaskNotification(NotificationType type);

    private:
        NotificationType m_type;
    };

    class BuildRunner;
    typedef void (*TaskNotificationFunc)(
        BuildRunner *runner,
        std::unique_ptr<TaskNotification> notification,
        void *data);

    class NotifyStateChanged : public TaskNotification {

    public:
        NotifyStateChanged(const TaskKey &key, const TaskState &state);
        TaskKey getKey() const;
        TaskState getState() const;
        std::string toString() const override;

    private:
        TaskKey m_key;
        TaskState m_state;
    };

    class NotifyTaskRequested : public TaskNotification {

    public:
        explicit NotifyTaskRequested(const TaskKey &requested);
        TaskKey getRequested() const;
        std::string toString() const override;

    private:
        TaskKey m_requested;
    };

    class NotifyTaskBlocked : public TaskNotification {

    public:
        NotifyTaskBlocked(const TaskKey &key, const absl::flat_hash_set<TaskKey> &dependencies);
        TaskKey getKey() const;
        absl::flat_hash_set<TaskKey> getDependencies() const;
        std::string toString() const override;

    private:
        TaskKey m_key;
        absl::flat_hash_set<TaskKey> m_dependencies;
    };

    class NotifyTaskUnblocked : public TaskNotification {

    public:
        explicit NotifyTaskUnblocked(const TaskKey &unblocked);
        TaskKey getUnblocked() const;
        std::string toString() const override;

    private:
        TaskKey m_unblocked;
    };

    class NotifyThreadCancelled : public TaskNotification {

    public:
        explicit NotifyThreadCancelled(int index);
        int getIndex() const;
        std::string toString() const override;

    private:
        int m_index;
    };
}

#endif // LYRIC_BUILD_TASK_NOTIFICATION_H
