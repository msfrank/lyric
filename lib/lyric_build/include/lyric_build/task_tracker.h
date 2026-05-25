#ifndef LYRIC_BUILD_TASK_TRACKER_H
#define LYRIC_BUILD_TASK_TRACKER_H

#include "base_task.h"

namespace lyric_build {

    class TaskTracker {
    public:
        TaskTracker();

        bool hasTask(const TaskKey &key) const;
        BaseTask *getTask(const TaskKey &key) const;

        TaskData loadState(const TaskKey &key);
        absl::flat_hash_map<TaskKey,TaskData> loadStates(const absl::flat_hash_set<TaskKey> &keys);
        absl::flat_hash_map<TaskKey,TaskData> loadStates(
            absl::flat_hash_set<TaskKey>::const_iterator begin,
            absl::flat_hash_set<TaskKey>::const_iterator end);

    private:
        std::unique_ptr<absl::Mutex> m_lock;
        absl::flat_hash_map<TaskKey, BaseTask *> m_tasks;
    };
}

#endif // LYRIC_BUILD_TASK_TRACKER_H
