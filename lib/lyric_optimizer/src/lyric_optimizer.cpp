
#include <lyric_optimizer/optimizer_result.h>
#include <lyric_optimizer/lyric_optimizer.h>

lyric_optimizer::LyricOptimizer::LyricOptimizer(
    lyric_assembler::ObjectState *objectState,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder,
    const OptimizerOptions &options)
    : m_objectState(objectState),
      m_recorder(recorder),
      m_options(options)
{
}

tempo_utils::Status
lyric_optimizer::LyricOptimizer::initialize()
{
    auto scopeManager = std::make_unique<tempo_tracing::ScopeManager>(m_recorder);
    m_scopeManager = std::move(scopeManager);
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
