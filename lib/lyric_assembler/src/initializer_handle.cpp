
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/initializer_handle.h>

lyric_assembler::InitializerHandle::InitializerHandle(const std::string &name, CallSymbol *callSymbol)
    : m_name(name),
      m_callSymbol(callSymbol)
{
    TU_ASSERT (!m_name.empty());
    TU_ASSERT (m_callSymbol != nullptr);

}

std::string
lyric_assembler::InitializerHandle::getName() const
{
    return m_name;
}

lyric_common::SymbolUrl
lyric_assembler::InitializerHandle::getSymbolUrl() const
{
    return m_callSymbol->getSymbolUrl();
}

lyric_assembler::AbstractResolver *
lyric_assembler::InitializerHandle::initializerResolver() const
{
    return m_callSymbol->callResolver();
}

lyric_assembler::TemplateHandle *
lyric_assembler::InitializerHandle::initializerTemplate() const
{
    return m_callSymbol->callTemplate();
}

const lyric_assembler::ProcHandle *
lyric_assembler::InitializerHandle::initializerProc() const
{
    return m_callSymbol->callProc();
}

lyric_assembler::TypeHandle *
lyric_assembler::InitializerHandle::initializerType()
{
    return m_callSymbol->callType();
}

lyric_assembler::ProcHandle *
lyric_assembler::InitializerHandle::initializerProc()
{
    return m_callSymbol->callProc();
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::InitializerHandle::finalizeInitializer()
{
    return m_callSymbol->finalizeCall();
}