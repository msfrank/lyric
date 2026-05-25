
#include <lyric_build/task_tracker.h>

lyric_build::TaskTracker::TaskTracker()
    : m_lock(std::make_unique<absl::Mutex>())
{
}

bool
lyric_build::TaskTracker::hasTask(const TaskKey &key) const
{
    absl::ReaderMutexLock locker(m_lock.get());
    return m_tasks.contains(key);
}

lyric_build::BaseTask *
lyric_build::TaskTracker::getTask(const TaskKey &key) const
{
    absl::ReaderMutexLock locker(m_lock.get());
    auto entry = m_tasks.find(key);
    if (entry != m_tasks.cend())
        return entry->second;
    return nullptr;
}

lyric_build::TaskData
lyric_build::TaskTracker::loadState(const TaskKey &key)
{
    absl::ReaderMutexLock locker(m_lock.get());

    auto entry = m_tasks.find(key);
    if (entry == m_tasks.cend())
        return {};
    auto *task = entry->second;
    return task->getData();
}

absl::flat_hash_map<lyric_build::TaskKey, lyric_build::TaskData>
lyric_build::TaskTracker::loadStates(const absl::flat_hash_set<TaskKey> &keys)
{
    return loadStates(keys.cbegin(), keys.cend());
}

absl::flat_hash_map<lyric_build::TaskKey, lyric_build::TaskData>
lyric_build::TaskTracker::loadStates(
    absl::flat_hash_set<TaskKey>::const_iterator begin,
    absl::flat_hash_set<TaskKey>::const_iterator end)
{
    absl::ReaderMutexLock locker(m_lock.get());

    absl::flat_hash_map<TaskKey,TaskData> states;
    for (; begin != end; begin++) {
        const auto &key = *begin;
        auto entry = m_tasks.find(key);
        if (entry != m_tasks.cend()) {
            auto *task = entry->second;
            states[key] = task->getData();
        }
    }
    return states;
}