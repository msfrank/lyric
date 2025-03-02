
#include <lyric_assembler/local_variable.h>

lyric_assembler::LocalVariable::LocalVariable(
    const lyric_common::SymbolUrl &localUrl,
    lyric_object::AccessType access,
    const lyric_common::TypeDef &assignableType,
    LocalOffset offset)
    : m_localUrl(localUrl),
      m_access(access),
      m_assignableType(assignableType),
      m_offset(offset)
{
}

bool
lyric_assembler::LocalVariable::isImported() const
{
    return false;
}

bool
lyric_assembler::LocalVariable::isCopied() const
{
    return false;
}

lyric_assembler::SymbolType
lyric_assembler::LocalVariable::getSymbolType() const
{
    return SymbolType::LOCAL;
}

lyric_common::SymbolUrl
lyric_assembler::LocalVariable::getSymbolUrl() const
{
    return m_localUrl;
}

lyric_common::TypeDef
lyric_assembler::LocalVariable::getTypeDef() const
{
    return m_assignableType;
}

std::string
lyric_assembler::LocalVariable::getName() const
{
    return m_localUrl.getSymbolPath().getName();
}

lyric_object::AccessType
lyric_assembler::LocalVariable::getAccessType() const
{
    return m_access;
}

lyric_assembler::LocalOffset
lyric_assembler::LocalVariable::getOffset() const
{
    return m_offset;
}
