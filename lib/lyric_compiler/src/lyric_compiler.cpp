
#include <lyric_assembler/object_state.h>
#include <lyric_compiler/compiler_scan_driver.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/entry_handler.h>
#include <lyric_compiler/lyric_compiler.h>
#include <lyric_parser/node_walker.h>
#include <lyric_rewriter/lyric_rewriter.h>

lyric_compiler::LyricCompiler::LyricCompiler(
    std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
    std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
    const CompilerOptions &options)
    : m_localModuleCache(std::move(localModuleCache)),
      m_systemModuleCache(std::move(systemModuleCache)),
      m_options(options)
{
    TU_ASSERT (m_systemModuleCache != nullptr);
}

lyric_compiler::LyricCompiler::LyricCompiler(const LyricCompiler &other)
    : m_localModuleCache(other.m_localModuleCache),
      m_systemModuleCache(other.m_systemModuleCache),
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
            location, m_localModuleCache, m_systemModuleCache, &scopeManager, objectStateOptions);

        // initialize the assembler
        lyric_assembler::ObjectRoot *root;
        TU_ASSIGN_OR_RETURN (root, objectState.defineRoot());

        // construct the driver
        auto compilerDriver = std::make_shared<CompilerScanDriver>(root, &objectState);
        auto rootHandler = std::make_unique<EntryHandler>(compilerDriver.get());
        TU_RETURN_IF_NOT_OK (compilerDriver->initialize(std::move(rootHandler)));

        lyric_rewriter::RewriterOptions rewriterOptions;
        lyric_rewriter::LyricRewriter rewriter(rewriterOptions);
        TU_RETURN_IF_NOT_OK (rewriter.scanArchetype(archetype, location.toUrl(), compilerDriver, recorder));

        // construct object from object state and return it
        return objectState.toObject();

    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    }
}