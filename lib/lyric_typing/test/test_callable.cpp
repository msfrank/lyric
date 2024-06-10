
#include "test_callable.h"

TestCallable::TestCallable(
    const std::vector<lyric_assembler::Parameter> &listParameters,
    const std::vector<lyric_assembler::Parameter> &namedParameters,
    const lyric_assembler::Parameter &restParameter)
    : m_templateHandle(nullptr),
      m_listParameters(listParameters),
      m_namedParameters(namedParameters),
      m_restParameter(restParameter)
{
}

TestCallable::TestCallable(
    const std::vector<lyric_assembler::Parameter> &listParameters,
    const std::vector<lyric_assembler::Parameter> &namedParameters,
    const lyric_assembler::Parameter &restParameter,
    lyric_assembler::TemplateHandle *templateHandle)
    : m_templateHandle(templateHandle),
      m_listParameters(listParameters),
      m_namedParameters(namedParameters),
      m_restParameter(restParameter)
{
    TU_ASSERT (templateHandle != nullptr);
}

lyric_assembler::TemplateHandle *
TestCallable::getTemplate() const
{
    return m_templateHandle;
}

std::vector<lyric_assembler::Parameter>::const_iterator
TestCallable::listPlacementBegin() const
{
    return m_listParameters.cbegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
TestCallable::listPlacementEnd() const
{
    return m_listParameters.cend();
}

std::vector<lyric_assembler::Parameter>::const_iterator
TestCallable::namedPlacementBegin() const
{
    return m_namedParameters.cbegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
TestCallable::namedPlacementEnd() const
{
    return m_namedParameters.cend();
}

const lyric_assembler::Parameter *
TestCallable::restPlacement() const
{
    return &m_restParameter;
}

bool
TestCallable::hasInitializer(const std::string &name) const
{
    return false;
}

lyric_common::SymbolUrl
TestCallable::getInitializer(const std::string &name) const
{
    return {};
}

tempo_utils::Result<lyric_common::TypeDef>
TestCallable::invoke(
    lyric_assembler::BlockHandle *block,
    const lyric_assembler::AbstractCallsiteReifier &reifier)
{
    return tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kUnimplemented);
}
