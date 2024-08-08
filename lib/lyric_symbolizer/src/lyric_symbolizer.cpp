
#include <lyric_assembler/assembly_state.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <lyric_symbolizer/lyric_symbolizer.h>
#include <lyric_symbolizer/symbolizer_result.h>
#include <lyric_symbolizer/symbolizer_scan_driver.h>
#include <tempo_utils/log_stream.h>

lyric_symbolizer::LyricSymbolizer::LyricSymbolizer(
    std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
    const SymbolizerOptions &options)
    : m_systemModuleCache(std::move(systemModuleCache)),
      m_options(options)
{
}

lyric_symbolizer::LyricSymbolizer::LyricSymbolizer(const LyricSymbolizer &other)
    : m_systemModuleCache(other.m_systemModuleCache),
      m_options(other.m_options)
{
}

tempo_utils::Result<lyric_object::LyricObject>
lyric_symbolizer::LyricSymbolizer::symbolizeModule(
    const lyric_common::AssemblyLocation &location,
    const lyric_parser::LyricArchetype &archetype,
    const lyric_assembler::AssemblyStateOptions &assemblyStateOptions,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder)
{
    if (!location.isValid())
        return SymbolizerStatus::forCondition(
            SymbolizerCondition::kSymbolizerInvariant, "invalid assembly location");

    auto walker = archetype.getRoot();
    if (!walker.isValid())
        return SymbolizerStatus::forCondition(
            SymbolizerCondition::kSymbolizerInvariant, "invalid archetype");

    try {

        // create a new span
        tempo_tracing::ScopeManager scopeManager(recorder);
        auto span = scopeManager.makeSpan();
        span->setOperationName("symbolizeModule");

        // construct the assembler state
        lyric_assembler::AssemblyState assemblyState(
            location, m_systemModuleCache, &scopeManager, assemblyStateOptions);

        // initialize the assembler
        TU_RETURN_IF_NOT_OK (assemblyState.initialize());

        auto symbolizerDriver = std::make_shared<SymbolizerScanDriver>(&assemblyState);

        lyric_rewriter::RewriterOptions rewriterOptions;
        lyric_rewriter::LyricRewriter rewriter(rewriterOptions);
        TU_RETURN_IF_NOT_OK (rewriter.scanArchetype(archetype, location.toUrl(), symbolizerDriver, recorder));

        // construct object from assembly state and return it
        return assemblyState.toAssembly();

    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    }
}
