#ifndef LYRIC_BUILD_ABSTRACT_TASK_NOTIFIER_H
#define LYRIC_BUILD_ABSTRACT_TASK_NOTIFIER_H

#include "build_runner.h"
#include "build_types.h"

namespace lyric_build {

    class AbstractTaskNotifier {
    public:
        virtual ~AbstractTaskNotifier() = default;

        virtual void onStateChanged(
            const BuildRunner *runner,
            const TaskKey &key,
            const TaskData &data) = 0;

        virtual void onTaskRequested(
            const BuildRunner *runner,
            const TaskKey &requested) = 0;

        virtual void onTaskBlocked(
            const BuildRunner *runner,
            const TaskKey &key,
            const absl::flat_hash_set<TaskKey> &dependencies) = 0;

        virtual void onTaskUnblocked(
            const BuildRunner *runner,
            const TaskKey &unblocked) = 0;
    };
}
#endif // LYRIC_BUILD_ABSTRACT_TASK_NOTIFIER_H
