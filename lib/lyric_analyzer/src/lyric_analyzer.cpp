
#include <lyric_analyzer/analyzer_result.h>
#include <lyric_analyzer/analyzer_scan_driver.h>
#include <lyric_analyzer/lyric_analyzer.h>
#include <lyric_assembler/assembly_state.h>
#include <lyric_parser/node_walker.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <tempo_utils/log_stream.h>

lyric_analyzer::LyricAnalyzer::LyricAnalyzer(
    std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
    const AnalyzerOptions &options)
    : m_systemModuleCache(std::move(systemModuleCache)),
      m_options(options)
{
}

lyric_analyzer::LyricAnalyzer::LyricAnalyzer(const LyricAnalyzer &other)
    : m_systemModuleCache(other.m_systemModuleCache),
      m_options(other.m_options)
{
}

tempo_utils::Result<lyric_object::LyricObject>
lyric_analyzer::LyricAnalyzer::analyzeModule(
    const lyric_common::AssemblyLocation &location,
    const lyric_parser::LyricArchetype &archetype,
    const lyric_assembler::AssemblyStateOptions &assemblyStateOptions,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder)
{
    if (!location.isValid())
        return AnalyzerStatus::forCondition(
            AnalyzerCondition::kAnalyzerInvariant, "invalid assembly location");

    auto walker = archetype.getRoot();
    if (!walker.isValid())
        return AnalyzerStatus::forCondition(
            AnalyzerCondition::kAnalyzerInvariant, "invalid archetype");

    lyric_object::LyricObject assembly;
    try {

        // create a new span
        tempo_tracing::ScopeManager scopeManager(recorder);
        auto span = scopeManager.makeSpan();
        span->setOperationName("analyzeModule");

        // construct the analyzer state
        lyric_assembler::AssemblyState assemblyState(
            location, m_systemModuleCache, &scopeManager, assemblyStateOptions);
        lyric_typing::TypeSystem typeSystem(&assemblyState);

        // initialize the assembler
        TU_RETURN_IF_NOT_OK (assemblyState.initialize());

        auto analyzerDriver = std::make_shared<AnalyzerScanDriver>(&assemblyState);
        TU_RETURN_IF_NOT_OK (analyzerDriver->initialize());

        lyric_rewriter::RewriterOptions rewriterOptions;
        lyric_rewriter::LyricRewriter rewriter(rewriterOptions);
        TU_RETURN_IF_NOT_OK (rewriter.scanArchetype(archetype, location.toUrl(), analyzerDriver, recorder));

//        // define the module entry point
//        internal::EntryPoint entryPoint(&assemblyState);
//        auto status = entryPoint.initialize(assemblyState.getLocation());
//        if (status.notOk())
//            return status;
//
//        auto analyzeModuleResult = analyze_module(walker, entryPoint);
//        if (analyzeModuleResult.isStatus())
//            return analyzeModuleResult.getStatus();
//        assembly = analyzeModuleResult.getResult();
//
//        TU_ASSERT (assembly.isValid());
//        return assembly;

        // construct object from assembly state and return it
        return assemblyState.toAssembly();

    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    }
}