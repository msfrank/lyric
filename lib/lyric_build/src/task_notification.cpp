#include <absl/strings/substitute.h>

#include <lyric_build/task_notification.h>
#include <lyric_build/build_types.h>

lyric_build::TaskNotification::TaskNotification() : m_type(lyric_build::NotificationType::INVALID)
{
}

lyric_build::TaskNotification::TaskNotification(lyric_build::NotificationType type) : m_type(type)
{
}

lyric_build::NotificationType
lyric_build::TaskNotification::getType() const
{
    return m_type;
}

lyric_build::NotifyStateChanged::NotifyStateChanged(const TaskKey &key, const TaskState &state)
    : lyric_build::TaskNotification(lyric_build::NotificationType::STATE_CHANGED),
      m_key(key),
      m_state(state)
{
}

lyric_build::TaskKey
lyric_build::NotifyStateChanged::getKey() const
{
    return m_key;
}

lyric_build::TaskState
lyric_build::NotifyStateChanged::getState() const
{
    return m_state;
}

std::string
lyric_build::NotifyStateChanged::toString() const
{
    return absl::Substitute("NotifyStateChanged(key=$0, state=$1)",
        m_key.toString(), m_state.toString());
}

lyric_build::NotifyTaskRequested::NotifyTaskRequested(const TaskKey &requested)
    : lyric_build::TaskNotification(lyric_build::NotificationType::TASK_REQUESTED),
      m_requested(requested)
{
}

lyric_build::TaskKey
lyric_build::NotifyTaskRequested::getRequested() const
{
    return m_requested;
}

std::string
lyric_build::NotifyTaskRequested::toString() const
{
    return absl::Substitute("NotifyTaskRequested(requested=$0)", m_requested.toString());
}

lyric_build::NotifyTaskBlocked::NotifyTaskBlocked(const TaskKey &key, const absl::flat_hash_set<TaskKey> &dependencies)
    : lyric_build::TaskNotification(lyric_build::NotificationType::TASK_BLOCKED),
      m_key(key)
{
    for (const auto &dep : dependencies) {
        m_dependencies.insert(dep);
    }
}

lyric_build::TaskKey
lyric_build::NotifyTaskBlocked::getKey() const
{
    return m_key;
}

absl::flat_hash_set<lyric_build::TaskKey>
lyric_build::NotifyTaskBlocked::getDependencies() const
{
    return m_dependencies;
}

std::string
lyric_build::NotifyTaskBlocked::toString() const
{
    std::string dependencies = "{";
    auto iterator = m_dependencies.cbegin();
    if (iterator != m_dependencies.cend()) {
        dependencies.append(iterator->toString());
        iterator++;
        for (; iterator != m_dependencies.cend(); iterator++) {
            dependencies.append(", ");
            dependencies.append(iterator->toString());
        }
    }
    dependencies.append("}");
    return absl::Substitute("NotifyTaskBlocked(key=$0, dependencies=$1)", m_key.toString(), dependencies);
}

lyric_build::NotifyTaskUnblocked::NotifyTaskUnblocked(const TaskKey &unblocked)
    : lyric_build::TaskNotification(lyric_build::NotificationType::TASK_UNBLOCKED),
      m_unblocked(unblocked)
{
}

lyric_build::TaskKey
lyric_build::NotifyTaskUnblocked::getUnblocked() const
{
    return m_unblocked;
}

std::string
lyric_build::NotifyTaskUnblocked::toString() const
{
    return absl::Substitute("NotifyTaskUnblocked(unblocked=$0)", m_unblocked.toString());
}

lyric_build::NotifyThreadCancelled::NotifyThreadCancelled(int index)
    : lyric_build::TaskNotification(NotificationType::THREAD_CANCELLED), m_index(index)
{
}

int
lyric_build::NotifyThreadCancelled::getIndex() const
{
    return m_index;
}

std::string
lyric_build::NotifyThreadCancelled::toString() const
{
    return absl::Substitute("NotifyThreadCancelled(index=$0)", m_index);
}
