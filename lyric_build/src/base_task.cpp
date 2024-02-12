
#include <boost/uuid/uuid_io.hpp>

#include <lyric_build/base_task.h>
#include <lyric_build/build_types.h>
#include <lyric_build/build_attrs.h>
#include <tempo_tracing/trace_recorder.h>
#include <tempo_tracing/tracing_result.h>
#include <tempo_tracing/tracing_schema.h>

lyric_build::BaseTask::BaseTask(
    const boost::uuids::uuid &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : m_generation(generation),
      m_key(key),
      m_span(span),
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

boost::uuids::uuid
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

std::shared_ptr<tempo_tracing::TraceSpan>
lyric_build::BaseTask::getSpan() const
{
    return m_span;
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

//void
//lyric_build::BaseTask::setSpan(std::shared_ptr<tempo_tracing::TraceSpan> span)
//{
//    m_span = span;
//    m_span->setOperationName(getId().toString());
//    m_span->setStartTime(absl::Now());
//    m_span->putTag(tempo_tracing::kOpentracingComponent, std::string("lyric_build"));
//    m_span->putTag(lyric_build::kLyricBuildGeneration, boost::uuids::to_string(m_generation));
//    m_span->putTag(lyric_build::kLyricBuildTaskParams, m_key.getParams().toString());
//}
//
//void
//lyric_build::BaseTask::putTag(const tempo_utils::Attr &tag)
//{
//    TU_ASSERT (m_span);
//    m_span->putTag(tag);
//}
//
//void
//lyric_build::BaseTask::putTags(std::initializer_list<tempo_utils::Attr> tags)
//{
//    TU_ASSERT (m_span);
//    m_span->putTags(tags);
//}
//
//void
//lyric_build::BaseTask::putLog(std::initializer_list<tempo_utils::Attr> fields)
//{
//    TU_ASSERT (m_span);
//    m_span->putLogs(fields);
//}
//
//void
//lyric_build::BaseTask::putError(std::error_code ec, std::string_view message)
//{
//    TU_ASSERT (m_span);
//    m_span->putTag(tempo_tracing::kOpentracingEvent, std::string("error"));
//    m_span->putTag(tempo_tracing::kOpentracingErrorKind, absl::StrCat(ec.category().name(), "|", ec.message()));
//    m_span->putTag(tempo_tracing::kOpentracingMessage, std::string(message));
//}

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

    // if this is the first iteration of the task then set the hash
    if (m_state == State::READY) {
        m_span->setStartTime(absl::Now());
        m_span->putTag(tempo_tracing::kOpentracingComponent, std::string("lyric_build"));
        m_span->putTag(lyric_build::kLyricBuildGeneration, boost::uuids::to_string(m_generation));
        m_span->putTag(lyric_build::kLyricBuildTaskParams, m_key.getParams().toString());
        m_span->putTag(lyric_build::kLyricBuildTaskHash, taskHash);
        m_state = State::ACTIVE;
    }

    // run the task
    auto statusOption = runTask(taskHash, depStates, buildState);
    m_span->setEndTime(absl::Now());

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

    return statusOption;
}