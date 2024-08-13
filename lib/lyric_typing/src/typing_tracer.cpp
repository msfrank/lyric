
#include <lyric_typing/typing_tracer.h>

lyric_typing::TypingTracer::TypingTracer(tempo_tracing::ScopeManager *scopeManager)
    : m_scopeManager(scopeManager)
{
    TU_ASSERT (m_scopeManager != nullptr);
}
