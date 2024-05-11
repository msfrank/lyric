
#include <lyric_build/build_diagnostics.h>
#include <lyric_build/build_result.h>
#include <tempo_tracing/tracing_schema.h>
#include <tempo_utils/log_helpers.h>

lyric_build::BuildDiagnostics::BuildDiagnostics(
    const tempo_tracing::TempoSpanset &spanset,
    const BuildDiagnosticsOptions &options)
    : m_spanset(spanset),
      m_options(options)
{
    TU_ASSERT (m_spanset.isValid());
}

tempo_utils::Result<std::shared_ptr<lyric_build::BuildDiagnostics>>
lyric_build::BuildDiagnostics::create(
    const tempo_tracing::TempoSpanset &spanset,
    const BuildDiagnosticsOptions &options)
{
    if (!spanset.isValid())
        return BuildStatus::forCondition(
            BuildCondition::kBuildInvariant, "invalid spanset");
    return std::shared_ptr<BuildDiagnostics>(new BuildDiagnostics(spanset, options));
}

tempo_tracing::TempoSpanset
lyric_build::BuildDiagnostics::getSpanset() const
{
    return m_spanset;
}

static void
print_span(const tempo_tracing::SpanWalker &spanWalker, int indent)
{
    tempo_utils::Status status;

    TU_ASSERT (spanWalker.isValid());

    bool error;
    status = spanWalker.parseTag(tempo_tracing::kOpentracingError, error);
    if (status.notOk() && !status.matchesCondition(tempo_tracing::TracingCondition::kMissingTag)) {
        return;
    }

    TU_CONSOLE_OUT << tempo_utils::Indent(indent)
                   << "task " << spanWalker.getOperationName()
                   << " ... "
                   << (error ? "failed" : "complete");

    for (int i = 0; i < spanWalker.numLogs(); i++) {
        auto logWalker = spanWalker.getLog(i);
        std::string message;
        status = logWalker.parseField(tempo_tracing::kOpentracingMessage, message);
        if (status.isOk()) {
            TU_CONSOLE_OUT << tempo_utils::Indent(indent + 2) << message;
        } else {
            if (!status.matchesCondition(tempo_tracing::TracingCondition::kMissingLog)) {
                TU_CONSOLE_ERR << "failed to pring log: " << status;
            }
        }
    }

    for (int i = 0; i < spanWalker.numChildren(); i++) {
        print_span(spanWalker.getChild(i), indent + 4);
    }
}

void
lyric_build::BuildDiagnostics::printDiagnostics() const
{
    auto rootWalker = m_spanset.getRoots();
    for (int i = 0; i < rootWalker.numRoots(); i++) {
        auto root = rootWalker.getRoot(i);
        auto taskId = TaskId::fromString(root.getOperationName());
        if (m_options.skipUnknownTargets && !m_options.targets.contains(taskId))
            continue;
        print_span(root, 0);
    }
}
