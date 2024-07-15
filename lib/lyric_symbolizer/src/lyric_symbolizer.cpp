
#include <lyric_assembler/assembly_state.h>
#include <lyric_parser/node_walker.h>
#include <lyric_symbolizer/internal/symbolize_module.h>
#include <lyric_symbolizer/internal/entry_point.h>
#include <lyric_symbolizer/lyric_symbolizer.h>
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

        // construct the compiler state
        lyric_assembler::AssemblyState assemblyState(
            location, m_systemModuleCache, &scopeManager, assemblyStateOptions);
        lyric_typing::TypeSystem typeSystem(&assemblyState);

        // initialize the assembler
        TU_RETURN_IF_NOT_OK (assemblyState.initialize());

        // define the module entry point
        lyric_symbolizer::internal::EntryPoint entryPoint(&assemblyState);
        auto status = entryPoint.initialize(assemblyState.getLocation());
        if (status.notOk())
            return status;

        // symbolize single module into assembly
        auto symbolizeModuleResult = symbolize_module(walker, entryPoint);
        if (symbolizeModuleResult.isStatus())
            return symbolizeModuleResult.getStatus();
        auto assembly = symbolizeModuleResult.getResult();

        TU_ASSERT (assembly.isValid());
        return assembly;

    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    }
}
