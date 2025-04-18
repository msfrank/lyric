
#include <lyric_optimizer/optimizer_result.h>
#include <lyric_optimizer/lyric_optimizer.h>

lyric_optimizer::LyricOptimizer::LyricOptimizer(
    std::unique_ptr<lyric_assembler::ObjectState> &&objectState,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder,
    const OptimizerOptions &options)
    : m_objectState(std::move(objectState)),
      m_recorder(recorder),
      m_scopeManager(std::make_unique<tempo_tracing::ScopeManager>(m_recorder)),
      m_options(options)
{
}

lyric_optimizer::LyricOptimizer::LyricOptimizer(
    const lyric_common::ModuleLocation &location,
    std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
    std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder,
    const OptimizerOptions &options)
    : m_recorder(std::move(recorder)),
      m_scopeManager(std::make_unique<tempo_tracing::ScopeManager>(m_recorder)),
      m_options(options)
{
    m_objectState = std::make_unique<lyric_assembler::ObjectState>(
        location,
        localModuleCache,
        systemModuleCache,
        m_scopeManager.get());
}

tempo_utils::Status
lyric_optimizer::LyricOptimizer::initialize()
{
    if (m_objectState->objectRoot() != nullptr)
        return {};
    TU_RETURN_IF_NOT_OK (m_objectState->load());
    return {};
}

tempo_utils::Status
lyric_optimizer::LyricOptimizer::optimizeCall(const lyric_common::SymbolPath &callPath)
{
    return {};
}

tempo_utils::Result<lyric_object::LyricObject>
lyric_optimizer::LyricOptimizer::toObject() const
{
    return m_objectState->toObject();
}
