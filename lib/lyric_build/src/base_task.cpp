
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_result.h>
#include <lyric_build/build_types.h>
#include <tempo_tracing/trace_recorder.h>
#include <tempo_tracing/tracing_schema.h>

lyric_build::BaseTask::BaseTask(
    const BuildGeneration &generation,
    const TaskKey &key,
    std::weak_ptr<BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : m_generation(generation),
      m_key(key),
      m_buildState(std::move(buildState)),
      m_span(std::move(span)),
      m_lock(std::make_unique<absl::Mutex>()),
      m_state(TaskState::INVALID)
{
    TU_ASSERT (m_key.isValid());
    TU_ASSERT (m_span != nullptr);
    m_span->setOperationName(getId().toString());
}

lyric_build::BaseTask::~BaseTask()
{
    if (m_span != nullptr) {
        m_span->close();
    }
}

lyric_build::BuildGeneration
lyric_build::BaseTask::getGeneration() const
{
    return m_generation;
}

lyric_build::TaskKey
lyric_build::BaseTask::getKey() const
{
    return m_key;
}

lyric_build::TaskId
lyric_build::BaseTask::getId() const
{
    return TaskId(m_key.getDomain(), m_key.getId());
}

lyric_build::TaskReference
lyric_build::BaseTask::getReference() const
{
    return TaskReference(m_generation, m_key);
}

tempo_config::ConfigMap
lyric_build::BaseTask::getParams() const
{
    return m_key.getParams();
}

std::shared_ptr<lyric_build::BuildState>
lyric_build::BaseTask::getBuildState() const
{
    auto buildState = m_buildState.lock();
    TU_NOTNULL (buildState);
    return buildState;
}

std::shared_ptr<tempo_tracing::TraceRecorder>
lyric_build::BaseTask::traceDiagnostics()
{
    if (m_diagnostics != nullptr)
        return m_diagnostics;
    m_diagnostics = tempo_tracing::TraceRecorder::create();
    auto traceId = m_diagnostics->traceId();
    m_span->putTag(tempo_tracing::kTempoTracingContinuationHi, traceId.getHi());
    m_span->putTag(tempo_tracing::kTempoTracingContinuationLo, traceId.getLo());
    return m_diagnostics;
}

lyric_build::TaskData
lyric_build::BaseTask::getData() const
{
    absl::ReaderMutexLock locker(m_lock.get());
    return TaskData(m_state, m_generation, m_hash.value_or(TaskHash{}));
}

lyric_build::TaskState
lyric_build::BaseTask::getState() const
{
    absl::ReaderMutexLock locker(m_lock.get());
    return m_state;
}

lyric_build::TaskData
lyric_build::BaseTask::setState(TaskState state)
{
    absl::WriterMutexLock locker(m_lock.get());
    m_state = state;
    return TaskData(m_state, m_generation, m_hash.value_or(TaskHash{}));
}

bool
lyric_build::BaseTask::hasHash() const
{
    absl::ReaderMutexLock locker(m_lock.get());
    return m_hash.has_value();
}

lyric_build::TaskHash
lyric_build::BaseTask::getHash() const
{
    absl::ReaderMutexLock locker(m_lock.get());
    return m_hash.value_or(TaskHash{});
}

lyric_build::TaskData
lyric_build::BaseTask::setHash(const TaskHash &taskHash)
{
    absl::WriterMutexLock locker(m_lock.get());
    TU_ASSERT (!m_hash.has_value());
    m_hash = taskHash;
    return TaskData(m_state, m_generation, taskHash);
}

void
lyric_build::BaseTask::requestDependency(const TaskKey &depKey)
{
    if (!m_completed.contains(depKey)) {
        m_dependencies.insert(depKey);
    }
}

bool
lyric_build::BaseTask::hasDependency(const TaskKey &depKey) const
{
    return m_dependencies.contains(depKey);
}

bool
lyric_build::BaseTask::dependenciesEmpty() const
{
    return m_dependencies.empty();
}

absl::flat_hash_set<lyric_build::TaskKey>::const_iterator
lyric_build::BaseTask::dependenciesBegin() const
{
    return m_dependencies.cbegin();
}

absl::flat_hash_set<lyric_build::TaskKey>::const_iterator
lyric_build::BaseTask::dependenciesEnd() const
{
    return m_dependencies.cend();
}

int
lyric_build::BaseTask::numDependencies() const
{
    return m_dependencies.size();
}

void
lyric_build::BaseTask::markCompleted(const TaskKey &depKey, const TaskData &depState)
{
    TU_ASSERT (m_dependencies.contains(depKey));
    TU_ASSERT (!m_completed.contains(depKey));
    m_dependencies.erase(depKey);
    m_completed[depKey] = depState;
}

bool
lyric_build::BaseTask::hasCompleted(const TaskKey &depKey) const
{
    return m_completed.contains(depKey);
}

lyric_build::TaskData
lyric_build::BaseTask::getCompleted(const TaskKey &depKey) const
{
    auto entry = m_completed.find(depKey);
    if (entry != m_completed.cend())
        return entry->second;
    return {};
}

absl::btree_map<lyric_build::TaskKey,lyric_build::TaskData>::const_iterator
lyric_build::BaseTask::completedBegin() const
{
    return m_completed.cbegin();
}

absl::btree_map<lyric_build::TaskKey,lyric_build::TaskData>::const_iterator
lyric_build::BaseTask::completedEnd() const
{
    return m_completed.cend();
}

int
lyric_build::BaseTask::numCompleted() const
{
    return m_completed.size();
}

tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>>
lyric_build::BaseTask::getContent(
    const TaskKey &key,
    const tempo_utils::UrlPath &path,
    bool followLinks)
{
    auto entry = m_completed.find(key);
    if (entry == m_completed.cend())
        return BuildStatus::forCondition(BuildCondition::kMissingInput,
            "missing input {}; dependent task {} is not available", path.toString(), key.toString());
    auto &data = entry->second;

    auto buildState = m_buildState.lock();
    if (buildState == nullptr)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid task {}; missing build state", m_key.toString());

    auto artifactCache = buildState->getArtifactCache();
    TraceId traceId(data.getHash(), key);
    BuildGeneration generation;
    TU_ASSIGN_OR_RETURN (generation, artifactCache->loadTrace(traceId));

    ArtifactId artifactId(generation, data.getHash(), path);
    if (followLinks)
        return artifactCache->loadContentFollowingLinks(artifactId);
    return artifactCache->loadContent(artifactId);
}

tempo_utils::Result<lyric_build::LyricMetadata>
lyric_build::BaseTask::getMetadata(
    const TaskKey &key,
    const tempo_utils::UrlPath &path,
    bool followLinks)
{
    auto entry = m_completed.find(key);
    if (entry == m_completed.cend())
        return BuildStatus::forCondition(BuildCondition::kMissingInput,
            "missing input {}; dependent task {} is not available", path.toString(), key.toString());
    auto &data = entry->second;

    auto buildState = m_buildState.lock();
    if (buildState == nullptr)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid task {}; missing build state", m_key.toString());

    auto artifactCache = buildState->getArtifactCache();
    TraceId traceId(data.getHash(), key);
    BuildGeneration generation;
    TU_ASSIGN_OR_RETURN (generation, artifactCache->loadTrace(traceId));

    ArtifactId artifactId(generation, data.getHash(), path);
    if (followLinks)
        return artifactCache->loadMetadataFollowingLinks(artifactId);
    return artifactCache->loadMetadata(artifactId);
}

tempo_utils::Status
lyric_build::BaseTask::storeArtifact(
    const tempo_utils::UrlPath &path,
    std::shared_ptr<const tempo_utils::ImmutableBytes> content,
    const LyricMetadata &metadata)
{
    auto buildState = m_buildState.lock();
    if (buildState == nullptr)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid task {}; missing build state", m_key.toString());

    if (!m_hash.has_value())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid task {}; missing task hash", m_key.toString());
    auto taskHash = m_hash.value();

    auto artifactCache = buildState->getArtifactCache();

    ArtifactId artifactId(m_generation, taskHash, path);
    TU_RETURN_IF_NOT_OK (artifactCache->declareArtifact(artifactId));
    TU_RETURN_IF_NOT_OK (artifactCache->storeMetadata(artifactId, metadata));
    TU_RETURN_IF_NOT_OK (artifactCache->storeContent(artifactId, std::move(content)));

    return {};
}

tempo_utils::Status
lyric_build::BaseTask::run()
{
    if (m_state == TaskState::COMPLETED || m_state == TaskState::FAILED)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid task {}; unexpected task state", m_key.toString());

    if (!m_hash.has_value())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid task {}; missing task hash", m_key.toString());
    auto taskHash = m_hash.value();

    auto buildState = m_buildState.lock();
    if (buildState == nullptr)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid task {}; missing build state", m_key.toString());

    // create the temp directory
    m_tempDirectory = std::make_unique<TempDirectory>(
        buildState->getTempRoot(), buildState->getGeneration(), taskHash);

    // set common tags
    m_span->putTag(tempo_tracing::kOpentracingComponent, std::string("lyric_build"));
    m_span->putTag(kLyricBuildGeneration, m_generation.toString());
    m_span->putTag(kLyricBuildTaskParams, m_key.getParams().toString());
    m_span->putTag(kLyricBuildTaskHash, taskHash.toString());
    m_state = TaskState::RUNNING;

    absl::flat_hash_map<TaskKey,TaskData> depStates(m_completed.begin(), m_completed.end());

    // run the task
    m_span->activate();
    auto status = runTask(m_tempDirectory.get());

    // clean up the task
    m_span->deactivate();
    TU_RETURN_IF_NOT_OK (this->complete(status));

    return {};
}

tempo_utils::Status
lyric_build::BaseTask::cancel()
{
    switch (m_state) {
        case TaskState::COMPLETED:
        case TaskState::FAILED:
            return {};
        default:
            return complete(BuildStatus::forCondition(
                BuildCondition::kTaskFailure, "task has been cancelled"));
    }
}

tempo_utils::Status
lyric_build::BaseTask::fail(const tempo_utils::Status &status)
{
    switch (m_state) {
        case TaskState::COMPLETED:
        case TaskState::FAILED:
            return {};
        default:
            if (status.notOk())
                return complete(status);
            return complete(BuildStatus::forCondition(
                BuildCondition::kTaskFailure, "unknown task failure"));
    }
}

tempo_utils::Status
lyric_build::BaseTask::complete(const tempo_utils::Status &status)
{
    m_state = status.isOk()? TaskState::COMPLETED : TaskState::FAILED;

    auto buildState = m_buildState.lock();
    if (buildState == nullptr)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid task {}; missing build state", m_key.toString());

    // if there is a valid diagnostics recorder, then store it
    if (m_diagnostics != nullptr) {
        m_diagnostics->close();
        tempo_tracing::TempoSpanset spanset;
        TU_ASSIGN_OR_RETURN (spanset, m_diagnostics->toSpanset());
        auto artifactCache = buildState->getArtifactCache();
        artifactCache->storeDiagnostics(getReference(), spanset);
    }

    // close the span
    if (status.notOk()) {
        m_span->logStatus(status, tempo_tracing::LogSeverity::kError);
    }
    m_span->close();

    // clean up the temp directory
    if (m_tempDirectory) {
        TU_RETURN_IF_NOT_OK (m_tempDirectory->cleanup());
    }

    return {};
}

std::shared_ptr<tempo_tracing::SpanLog>
lyric_build::BaseTask::logInfo(std::string_view message)
{
    return m_span->logMessage(message, absl::Now(), tempo_tracing::LogSeverity::kInfo);
}

std::shared_ptr<tempo_tracing::SpanLog>
lyric_build::BaseTask::logWarn(std::string_view message)
{
    return m_span->logMessage(message, absl::Now(), tempo_tracing::LogSeverity::kWarn);
}

std::shared_ptr<tempo_tracing::SpanLog>
lyric_build::BaseTask::logError(std::string_view message)
{
    return m_span->logMessage(message, absl::Now(), tempo_tracing::LogSeverity::kError);
}

std::shared_ptr<tempo_tracing::SpanLog>
lyric_build::BaseTask::logStatus(const tempo_utils::Status &status)
{
    return m_span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
}
