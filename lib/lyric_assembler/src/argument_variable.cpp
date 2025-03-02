
#include <lyric_assembler/argument_variable.h>

lyric_assembler::ArgumentVariable::ArgumentVariable(
    const lyric_common::SymbolUrl &argumentUrl,
    const lyric_common::TypeDef &assignableType,
    BindingType bindingType,
    ArgumentOffset offset)
    : m_argumentUrl(argumentUrl),
      m_assignableType(assignableType),
      m_bindingType(bindingType),
      m_offset(offset)
{
}

bool
lyric_assembler::ArgumentVariable::isImported() const
{
    return false;
}

bool
lyric_assembler::ArgumentVariable::isCopied() const
{
    return false;
}

lyric_assembler::SymbolType
lyric_assembler::ArgumentVariable::getSymbolType() const
{
    return SymbolType::ARGUMENT;
}

lyric_common::SymbolUrl
lyric_assembler::ArgumentVariable::getSymbolUrl() const
{
    return m_argumentUrl;
}

lyric_common::TypeDef
lyric_assembler::ArgumentVariable::getTypeDef() const
{
    return m_assignableType;
}

std::string
lyric_assembler::ArgumentVariable::getName() const
{
    return m_argumentUrl.getSymbolPath().getName();
}

lyric_assembler::BindingType
lyric_assembler::ArgumentVariable::getBindingType() const
{
    return m_bindingType;
}

lyric_assembler::ArgumentOffset
lyric_assembler::ArgumentVariable::getOffset() const
{
    return m_offset;
}
