
#include <lyric_assembler/assembly_state.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/node_walker.h>
#include <lyric_rewriter/internal/rewrite_archetype.h>
#include <lyric_rewriter/internal/entry_point.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <lyric_rewriter/rewriter_result.h>
#include <tempo_utils/log_stream.h>

lyric_rewriter::LyricRewriter::LyricRewriter(
    std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
    const RewriterOptions &options)
    : m_systemModuleCache(std::move(systemModuleCache)),
      m_options(options)
{
}

lyric_rewriter::LyricRewriter::LyricRewriter(const LyricRewriter &other)
    : m_systemModuleCache(other.m_systemModuleCache),
      m_options(other.m_options)
{
}

tempo_utils::Result<lyric_parser::LyricArchetype>
lyric_rewriter::LyricRewriter::rewriteArchetype(
    const lyric_parser::LyricArchetype &archetype,
    const tempo_utils::Url &sourceUrl,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder)
{
    if (!sourceUrl.isValid())
        return RewriterStatus::forCondition(
            RewriterCondition::kRewriterInvariant, "invalid source url");

    auto walker = archetype.getNode(0);
    if (!walker.isValid())
        return RewriterStatus::forCondition(
            RewriterCondition::kRewriterInvariant, "invalid archetype");

    try {

        // create a new span
        tempo_tracing::ScopeManager scopeManager(recorder);
        auto span = scopeManager.makeSpan();
        span->setOperationName("rewriteArchetype");

        lyric_parser::ArchetypeState archetypeState(sourceUrl, &scopeManager);

        // define the archetype entry point
        lyric_rewriter::internal::EntryPoint entryPoint(&archetypeState);

        // rewrite archetype
        auto symbolizeModuleResult = rewrite_root(walker, entryPoint);
        if (symbolizeModuleResult.isStatus())
            return symbolizeModuleResult.getStatus();
        auto assembly = symbolizeModuleResult.getResult();

        TU_ASSERT (assembly.isValid());
        return assembly;

    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    }
}
