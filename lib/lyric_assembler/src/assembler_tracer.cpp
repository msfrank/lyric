
#include <lyric_assembler/assembler_tracer.h>

lyric_assembler::AssemblerTracer::AssemblerTracer(tempo_tracing::ScopeManager *scopeManager)
    : m_scopeManager(scopeManager)
{
    TU_ASSERT (m_scopeManager != nullptr);
}