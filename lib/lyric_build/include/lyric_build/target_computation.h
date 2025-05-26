#ifndef LYRIC_BUILD_TARGET_COMPUTATION_H
#define LYRIC_BUILD_TARGET_COMPUTATION_H

#include <lyric_build/build_diagnostics.h>
#include <lyric_build/build_types.h>

namespace lyric_build {

    class TargetComputation {
    public:
        TargetComputation();
        TargetComputation(const TaskId &id, const TaskState &state);
        TargetComputation(const TargetComputation &other);

        bool isValid() const;

        TaskId getId() const;
        TaskState getState() const;

    private:
        TaskId m_id;
        TaskState m_state;
    };

    class TargetComputationSet {
    public:
        TargetComputationSet();
        TargetComputationSet(
            const BuildGeneration &buildGen,
            const absl::flat_hash_map<TaskId, TargetComputation> &targetComputations,
            int totalTasksCreated,
            int totalTasksCached,
            std::chrono::milliseconds elapsedTime,
            std::shared_ptr<BuildDiagnostics> diagnostics);
        TargetComputationSet(const TargetComputationSet &other);

        bool isValid() const;

        BuildGeneration getBuildGeneration() const;
        int getTotalTasksCreated() const;
        int getTotalTasksCached() const;
        std::chrono::milliseconds getElapsedTime() const;

        bool containsTarget(const TaskId &target) const;
        TargetComputation getTarget(const TaskId &target) const;
        absl::flat_hash_map<TaskId, TargetComputation>::const_iterator targetsBegin() const;
        absl::flat_hash_map<TaskId, TargetComputation>::const_iterator targetsEnd() const;
        int numTargets() const;

        std::shared_ptr<BuildDiagnostics> getDiagnostics() const;

    private:
        BuildGeneration m_buildGen;
        absl::flat_hash_map<TaskId, TargetComputation> m_targetComputations;
        int m_totalTasksCreated = 0;
        int m_totalTasksCached = 0;
        std::chrono::milliseconds m_elapsedTime;
        std::shared_ptr<BuildDiagnostics> m_diagnostics;
    };
}

#endif // LYRIC_BUILD_TARGET_COMPUTATION_H