
#include <lyric_analyzer/analyzer_result.h>
#include <lyric_analyzer/analyzer_scan_driver.h>
#include <lyric_analyzer/lyric_analyzer.h>
#include <lyric_assembler/object_state.h>
#include <lyric_parser/node_walker.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <tempo_utils/log_stream.h>

lyric_analyzer::LyricAnalyzer::LyricAnalyzer(
    std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
    std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
    const AnalyzerOptions &options)
    : m_localModuleCache(std::move(localModuleCache)),
      m_systemModuleCache(std::move(systemModuleCache)),
      m_options(options)
{
}

lyric_analyzer::LyricAnalyzer::LyricAnalyzer(const LyricAnalyzer &other)
    : m_localModuleCache(other.m_localModuleCache),
      m_systemModuleCache(other.m_systemModuleCache),
      m_options(other.m_options)
{
}

tempo_utils::Result<lyric_object::LyricObject>
lyric_analyzer::LyricAnalyzer::analyzeModule(
    const lyric_common::ModuleLocation &location,
    const lyric_parser::LyricArchetype &archetype,
    const lyric_assembler::ObjectStateOptions &objectStateOptions,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder)
{
    if (!location.isValid())
        return AnalyzerStatus::forCondition(
            AnalyzerCondition::kAnalyzerInvariant, "invalid module location");

    auto walker = archetype.getRoot();
    if (!walker.isValid())
        return AnalyzerStatus::forCondition(
            AnalyzerCondition::kAnalyzerInvariant, "invalid archetype");

    try {

        // create a new span
        tempo_tracing::ScopeManager scopeManager(recorder);
        auto span = scopeManager.makeSpan();
        span->setOperationName("analyzeModule");

        // construct the analyzer state
        lyric_assembler::ObjectState objectState(
            location, m_localModuleCache, m_systemModuleCache, &scopeManager, objectStateOptions);
        lyric_typing::TypeSystem typeSystem(&objectState);

        // initialize the assembler
        lyric_assembler::ObjectRoot *root;
        TU_ASSIGN_OR_RETURN (root, objectState.defineRoot());

        auto analyzerDriver = std::make_shared<AnalyzerScanDriver>(root, &objectState);
        TU_RETURN_IF_NOT_OK (analyzerDriver->initialize());

        lyric_rewriter::RewriterOptions rewriterOptions;
        lyric_rewriter::LyricRewriter rewriter(rewriterOptions);
        TU_RETURN_IF_NOT_OK (rewriter.scanArchetype(archetype, location.toUrl(), analyzerDriver, recorder));

        // construct object from object state and return it
        return objectState.toObject();

    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    }
}