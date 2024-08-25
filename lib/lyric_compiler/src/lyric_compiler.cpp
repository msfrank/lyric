
#include <lyric_assembler/object_state.h>
#include <lyric_compiler/compiler_scan_driver.h>
#include <lyric_compiler/internal/compile_module.h>
#include <lyric_compiler/lyric_compiler.h>
#include <lyric_parser/node_walker.h>
#include <tempo_utils/log_stream.h>

lyric_compiler::LyricCompiler::LyricCompiler(
    std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
    const CompilerOptions &options)
    : m_systemModuleCache(systemModuleCache),
      m_options(options)
{
    TU_ASSERT (m_systemModuleCache != nullptr);
}

lyric_compiler::LyricCompiler::LyricCompiler(const LyricCompiler &other)
    : m_systemModuleCache(other.m_systemModuleCache),
      m_options(other.m_options)
{
}

tempo_utils::Result<lyric_object::LyricObject>
lyric_compiler::LyricCompiler::compileModule(
    const lyric_common::ModuleLocation &location,
    const lyric_parser::LyricArchetype &archetype,
    const lyric_assembler::ObjectStateOptions &objectStateOptions,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder)
{
    if (!location.isValid())
        return CompilerStatus::forCondition(
            CompilerCondition::kCompilerInvariant, "invalid module location");

    auto walker = archetype.getRoot();
    if (!walker.isValid())
        return CompilerStatus::forCondition(
            CompilerCondition::kCompilerInvariant, "invalid archetype");

    try {

        // create a new span
        tempo_tracing::ScopeManager scopeManager(recorder);
        auto span = scopeManager.makeSpan();
        span->setOperationName("compileModule");

        // construct the compiler state
        lyric_assembler::ObjectState objectState(
            location, m_systemModuleCache, &scopeManager, objectStateOptions);

        // initialize the assembler
        TU_RETURN_IF_NOT_OK (objectState.initialize());

        auto compilerDriver = std::make_shared<CompilerScanDriver>(&objectState);

//        // load env symbols into the assembly state
//        for (auto iterator = m_options.envSymbols.cbegin(); iterator != m_options.envSymbols.cend(); iterator++) {
//            status = bootstrap_single_assembly(iterator->first, state, iterator->second);
//            if (!status.isOk())
//                return status;
//        }

        // define the module entry point
        ModuleEntry moduleEntry(&objectState);
        TU_RETURN_IF_NOT_OK (moduleEntry.initialize());

        // compile single module into object
        lyric_object::LyricObject object;
        TU_ASSIGN_OR_RETURN (object, internal::compile_module(walker, moduleEntry, m_options.touchExternalSymbols));

        TU_ASSERT (object.isValid());
        return object;

    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    }
}