
#include <lyric_build/base_task.h>
#include <lyric_build/build_types.h>
#include <lyric_build/build_attrs.h>
#include <tempo_tracing/trace_recorder.h>
#include <tempo_tracing/tracing_result.h>
#include <tempo_tracing/tracing_schema.h>

lyric_build::BaseTask::BaseTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : m_generation(generation),
      m_key(key),
      m_span(std::move(span)),
      m_state(State::READY)
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

tempo_utils::UUID
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

tempo_config::ConfigMap
lyric_build::BaseTask::getParams() const
{
    return m_key.getParams();
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

lyric_build::TempDirectory *
lyric_build::BaseTask::tempDirectory()
{
    return m_tempDirectory.get();
}

Option<tempo_utils::Status>
lyric_build::BaseTask::run(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    if (m_state == State::DONE)
        return Option<tempo_utils::Status>(
            BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "task {} is already complete", m_key.toString()));

    // if the temp directory doesn't exist then create it but don't initialize until needed
    if (m_tempDirectory == nullptr) {
        m_tempDirectory = std::make_unique<TempDirectory>(
            buildState->getTempRoot(), buildState->getGeneration(), taskHash);
    }

    // if this is the first iteration of the task then set the hash
    if (m_state == State::READY) {
        m_span->putTag(tempo_tracing::kOpentracingComponent, std::string("lyric_build"));
        m_span->putTag(kLyricBuildGeneration, m_generation.toString());
        m_span->putTag(kLyricBuildTaskParams, m_key.getParams().toString());
        m_span->putTag(kLyricBuildTaskHash, taskHash);
        m_state = State::ACTIVE;
    }

    // run the task
    m_span->activate();
    auto statusOption = runTask(taskHash, depStates, buildState);
    m_span->deactivate();

    // signal to manager if the task is incomplete
    if (statusOption.isEmpty() )
        return statusOption;

    // otherwise the task has completed (successfully or with error)
    // TODO: should we handle retryable status differently?
    m_state = State::DONE;

    // if there is a valid diagnostics recorder, then store it
    if (m_diagnostics != nullptr) {
        m_diagnostics->close();
        auto toSpansetResult = m_diagnostics->toSpanset();
        if (toSpansetResult.isStatus()) {
            m_span->logStatus(toSpansetResult.getStatus(), absl::Now(), tempo_tracing::LogSeverity::kWarn);
        } else {
            auto spanset = toSpansetResult.getResult();
            auto cache = buildState->getCache();
            cache->storeDiagnostics(TraceId(taskHash, m_key.getDomain(), m_key.getId()), spanset);
        }
    }

    // close the span
    auto status = statusOption.getValue();
    if (status.notOk()) {
        m_span->logStatus(status, tempo_tracing::LogSeverity::kError);
    }
    m_span->close();

    // clean up the temp directory
    if (m_tempDirectory) {
        status = m_tempDirectory->cleanup();
        if (status.notOk())
            return Option(status);
    }

    return statusOption;
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
