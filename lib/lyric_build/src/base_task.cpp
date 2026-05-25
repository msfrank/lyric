
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
      m_state(TaskState::New)
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

tempo_utils::Result<lyric_build::TaskData>
lyric_build::BaseTask::setState(TaskState state)
{
    absl::WriterMutexLock locker(m_lock.get());

    if (!m_owner.has_value() || m_owner.value() != uv_thread_self())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid transition for task {}; task is not locked by the current thread", m_key.toString());

    // verify new state is valid to transition to
    switch (state) {
        case TaskState::Invalid:
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "invalid transition for task {}; cannot transition to Invalid state", m_key.toString());
        case TaskState::New:
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "invalid transition for task {}; cannot transition to New state", m_key.toString());
        default:
            break;
    }
    // verify old state is valid to transition from
    switch (m_state) {
        case TaskState::Completed:
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "invalid transition for task {}; task is already Completed", m_key.toString());
        case TaskState::Failed:
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "invalid transition for task {}; task is already Failed", m_key.toString());
        default:
            break;
    }
    TU_LOG_VV << "transitioning " << m_key << " from " << m_state << " to " << state;
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

tempo_utils::Result<lyric_build::TaskData>
lyric_build::BaseTask::setHash(const TaskHash &taskHash)
{
    absl::WriterMutexLock locker(m_lock.get());
    if (!m_owner.has_value() || m_owner.value() != uv_thread_self())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid transition for task {}; task is not locked by the current thread", m_key.toString());
    if (m_hash.has_value())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid transition for task {}; task hash is already set", m_key.toString());
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

    TU_LOG_V << "task " << m_key << " stores " << path << " at " << artifactId;

    return {};
}

tempo_utils::Status
lyric_build::BaseTask::linkArtifact(
    const TaskKey &key,
    const tempo_utils::UrlPath &path,
    const tempo_utils::UrlPath &overrideDestinationPath)
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

    if (!m_hash.has_value())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid task {}; missing task hash", m_key.toString());
    auto taskHash = m_hash.value();

    auto dstPath = overrideDestinationPath.isValid()? overrideDestinationPath : path;

    auto artifactCache = buildState->getArtifactCache();
    TraceId traceId(data.getHash(), key);
    BuildGeneration generation;
    TU_ASSIGN_OR_RETURN (generation, artifactCache->loadTrace(traceId));

    ArtifactId srcId(generation, data.getHash(), path);
    ArtifactId dstId(getGeneration(), taskHash, dstPath);

    TU_LOG_V << "task " << m_key << " links " << srcId << " to " << dstId;

    return artifactCache->linkArtifact(dstId, srcId);
}

tempo_utils::Status
lyric_build::BaseTask::linkArtifactOverridingMetadata(
    const TaskKey &key,
    const tempo_utils::UrlPath &path,
    const LyricMetadata &overrideMetadata,
    const tempo_utils::UrlPath &overrideDestinationPath)
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

    if (!m_hash.has_value())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid task {}; missing task hash", m_key.toString());
    auto taskHash = m_hash.value();

    auto dstPath = overrideDestinationPath.isValid()? overrideDestinationPath : path;

    auto artifactCache = buildState->getArtifactCache();
    TraceId traceId(data.getHash(), key);
    BuildGeneration generation;
    TU_ASSIGN_OR_RETURN (generation, artifactCache->loadTrace(traceId));

    ArtifactId srcId(generation, data.getHash(), path);
    ArtifactId dstId(getGeneration(), taskHash, dstPath);

    TU_LOG_V << "task " << m_key << " links " << srcId << " to " << dstId << " overriding metadata";

    return artifactCache->linkArtifactOverridingMetadata(dstId, overrideMetadata, srcId);
}

tempo_utils::Status
lyric_build::BaseTask::run(tempo_utils::Status &taskStatus)
{
    if (m_state == TaskState::Completed || m_state == TaskState::Failed)
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

    // run the task
    m_span->activate();
    taskStatus = runTask(m_tempDirectory.get());

    // clean up the task
    m_span->deactivate();

    return {};
}

tempo_utils::Status
lyric_build::BaseTask::close()
{
    m_span->close();

    // remove the temp directory
    tempo_utils::Status removeTempdirStatus;
    if (m_tempDirectory) {
        removeTempdirStatus = m_tempDirectory->cleanup();
    }

    // if there is a valid diagnostics recorder, then store it
    tempo_utils::Status storeDiagnosticsStatus;
    if (m_diagnostics != nullptr) {
        m_diagnostics->close();

        auto buildState = m_buildState.lock();
        if (buildState != nullptr) {

            auto toSpansetResult = m_diagnostics->toSpanset();
            if (toSpansetResult.isResult()) {
                auto spanset = toSpansetResult.getResult();
                auto artifactCache = buildState->getArtifactCache();
                storeDiagnosticsStatus = artifactCache->storeDiagnostics(getReference(), spanset);
            } else {
                storeDiagnosticsStatus = toSpansetResult.getStatus();
            }

        } else {
            storeDiagnosticsStatus =  BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "invalid task {}; missing build state", m_key.toString());
        }
    }

    // report any errors after we have finished cleanup
    TU_RETURN_IF_NOT_OK (removeTempdirStatus);
    TU_RETURN_IF_NOT_OK (storeDiagnosticsStatus);
    return {};
}

std::shared_ptr<tempo_tracing::SpanLog>
lyric_build::BaseTask::logInfo(std::string_view message)
{
    TU_LOG_V << "task " << m_key << " (INFO): " << message;
    return m_span->logMessage(message, absl::Now(), tempo_tracing::LogSeverity::kInfo);
}

std::shared_ptr<tempo_tracing::SpanLog>
lyric_build::BaseTask::logWarn(std::string_view message)
{
    TU_LOG_V << "task " << m_key << " (WARN): " << message;
    return m_span->logMessage(message, absl::Now(), tempo_tracing::LogSeverity::kWarn);
}

std::shared_ptr<tempo_tracing::SpanLog>
lyric_build::BaseTask::logError(std::string_view message)
{
    TU_LOG_V << "task " << m_key << " (ERROR): " << message;
    return m_span->logMessage(message, absl::Now(), tempo_tracing::LogSeverity::kError);
}

std::shared_ptr<tempo_tracing::SpanLog>
lyric_build::BaseTask::logStatus(const tempo_utils::Status &status)
{
    TU_LOG_V << "task " << m_key << " (ERROR): " << status;
    return m_span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
}

lyric_build::TaskLocker::TaskLocker(BaseTask *task)
    : m_task(task)
{
    TU_NOTNULL (m_task);
    absl::WriterMutexLock lock(m_task->m_lock.get());
    if (m_task->m_owner.has_value()) {
        TU_ASSERT (m_task->m_owner.value() != uv_thread_self());
        m_locked = false;
    } else {
        m_task->m_owner = uv_thread_self();
        m_locked = true;
        TU_LOG_V << "task " << m_task->getKey() << " locked by thread " << uv_thread_self();
    }
}

lyric_build::TaskLocker::~TaskLocker()
{
    absl::WriterMutexLock lock(m_task->m_lock.get());
    if (m_locked) {
        TU_ASSERT (m_task->m_owner.value() == uv_thread_self());
        m_task->m_owner.reset();
        TU_LOG_V << "task " << m_task->getKey() << " unlocked by thread " << uv_thread_self();
    }
}

bool
lyric_build::TaskLocker::isLocked() const
{
    return m_locked;
}
