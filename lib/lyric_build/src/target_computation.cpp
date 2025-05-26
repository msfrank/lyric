
#include <lyric_build/build_types.h>
#include <lyric_build/target_computation.h>
#include <tempo_utils/log_stream.h>

lyric_build::TargetComputation::TargetComputation()
{
}

lyric_build::TargetComputation::TargetComputation(const TaskId &id, const TaskState &state)
    : m_id(id),
      m_state(state)
{
}

lyric_build::TargetComputation::TargetComputation(const TargetComputation &other)
    : m_id(other.m_id),
      m_state(other.m_state)
{
}

bool
lyric_build::TargetComputation::isValid() const
{
    return m_id.isValid();
}

lyric_build::TaskId
lyric_build::TargetComputation::getId() const
{
    return m_id;
}

lyric_build::TaskState
lyric_build::TargetComputation::getState() const
{
    return m_state;
}

lyric_build::TargetComputationSet::TargetComputationSet()
    : m_totalTasksCreated(0),
      m_totalTasksCached(0),
      m_elapsedTime(0)
{
}

lyric_build::TargetComputationSet::TargetComputationSet(
    const BuildGeneration &buildGen,
    const absl::flat_hash_map<TaskId, TargetComputation> &targetComputations,
    int totalTasksCreated,
    int totalTasksCached,
    std::chrono::milliseconds elapsedTime,
    std::shared_ptr<BuildDiagnostics> diagnostics)
    : m_buildGen(buildGen),
      m_targetComputations(targetComputations),
      m_totalTasksCreated(totalTasksCreated),
      m_totalTasksCached(totalTasksCached),
      m_elapsedTime(elapsedTime),
      m_diagnostics(diagnostics)
{
}

lyric_build::TargetComputationSet::TargetComputationSet(const TargetComputationSet &other)
    : m_buildGen(other.m_buildGen),
      m_targetComputations(other.m_targetComputations),
      m_totalTasksCreated(other.m_totalTasksCreated),
      m_totalTasksCached(other.m_totalTasksCached),
      m_elapsedTime(other.m_elapsedTime),
      m_diagnostics(other.m_diagnostics)
{
}

bool
lyric_build::TargetComputationSet::isValid() const
{
    return !m_targetComputations.empty();
}

lyric_build::BuildGeneration
lyric_build::TargetComputationSet::getBuildGeneration() const
{
    return m_buildGen;
}

int
lyric_build::TargetComputationSet::getTotalTasksCreated() const
{
    return m_totalTasksCreated;
}

int
lyric_build::TargetComputationSet::getTotalTasksCached() const
{
    return m_totalTasksCached;
}

std::chrono::milliseconds
lyric_build::TargetComputationSet::getElapsedTime() const
{
    return m_elapsedTime;
}

bool
lyric_build::TargetComputationSet::containsTarget(const TaskId &target) const
{
    return m_targetComputations.contains(target);
}

lyric_build::TargetComputation
lyric_build::TargetComputationSet::getTarget(const TaskId &target) const
{
    if (m_targetComputations.contains(target))
        return m_targetComputations.at(target);
    return {};
}

absl::flat_hash_map<lyric_build::TaskId, lyric_build::TargetComputation>::const_iterator
lyric_build::TargetComputationSet::targetsBegin() const
{
    return m_targetComputations.cbegin();
}

absl::flat_hash_map<lyric_build::TaskId, lyric_build::TargetComputation>::const_iterator
lyric_build::TargetComputationSet::targetsEnd() const
{
    return m_targetComputations.cend();
}

int
lyric_build::TargetComputationSet::numTargets() const
{
    return m_targetComputations.size();
}

std::shared_ptr<lyric_build::BuildDiagnostics>
lyric_build::TargetComputationSet::getDiagnostics() const
{
    return m_diagnostics;
}