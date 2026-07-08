
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/entry_handle.h>

lyric_assembler::EntryHandle::EntryHandle(const std::string &name, CallSymbol *callSymbol)
    : m_name(name),
      m_callSymbol(callSymbol)
{
    TU_ASSERT (!m_name.empty());
    TU_ASSERT (m_callSymbol != nullptr);

}

std::string
lyric_assembler::EntryHandle::getName() const
{
    return m_name;
}

lyric_common::SymbolUrl
lyric_assembler::EntryHandle::getSymbolUrl() const
{
    return m_callSymbol->getSymbolUrl();
}

lyric_assembler::AbstractResolver *
lyric_assembler::EntryHandle::entryResolver() const
{
    return m_callSymbol->callResolver();
}

const lyric_assembler::ProcHandle *
lyric_assembler::EntryHandle::entryProc() const
{
    return m_callSymbol->callProc();
}

lyric_assembler::ProcHandle *
lyric_assembler::EntryHandle::entryProc()
{
    return m_callSymbol->callProc();
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::EntryHandle::finalizeEntry()
{
    return m_callSymbol->finalizeCall();
}
