
#include "test_callable.h"

TestCallable::TestCallable(
    const std::vector<lyric_assembler::Parameter> &listParameters,
    const std::vector<lyric_assembler::Parameter> &namedParameters,
    const lyric_assembler::Parameter &restParameter,
    const lyric_common::TypeDef &returnType)
    : m_listParameters(listParameters),
      m_namedParameters(namedParameters),
      m_restParameter(restParameter),
      m_returnType(returnType)
{
}

TestCallable::TestCallable(
    const std::vector<lyric_assembler::Parameter> &listParameters,
    const std::vector<lyric_assembler::Parameter> &namedParameters,
    const lyric_assembler::Parameter &restParameter,
    lyric_assembler::TemplateHandle *templateHandle,
    const lyric_common::TypeDef &returnType)
    : m_templateHandle(templateHandle),
      m_listParameters(listParameters),
      m_namedParameters(namedParameters),
      m_restParameter(restParameter),
      m_returnType(returnType)
{
    TU_ASSERT (m_templateHandle != nullptr);
}

TestCallable::TestCallable(
    const std::vector<lyric_assembler::Parameter> &listParameters,
    const std::vector<lyric_assembler::Parameter> &namedParameters,
    const lyric_assembler::Parameter &restParameter,
    const lyric_common::SymbolUrl &receiverUrl,
    const lyric_common::TypeDef &returnType)
    : m_receiverUrl(receiverUrl),
      m_listParameters(listParameters),
      m_namedParameters(namedParameters),
      m_restParameter(restParameter),
      m_returnType(returnType)
{
    TU_ASSERT (m_receiverUrl.isValid());
}

TestCallable::TestCallable(
    const std::vector<lyric_assembler::Parameter> &listParameters,
    const std::vector<lyric_assembler::Parameter> &namedParameters,
    const lyric_assembler::Parameter &restParameter,
    lyric_assembler::TemplateHandle *templateHandle,
    const lyric_common::SymbolUrl &receiverUrl,
    const lyric_common::TypeDef &returnType)
    : m_templateHandle(templateHandle),
      m_receiverUrl(receiverUrl),
      m_listParameters(listParameters),
      m_namedParameters(namedParameters),
      m_restParameter(restParameter),
      m_returnType(returnType)
{
    TU_ASSERT (m_templateHandle != nullptr);
    TU_ASSERT (m_receiverUrl.isValid());
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

bool
TestCallable::hasReceiver() const
{
    return m_receiverUrl.isValid();
}

lyric_common::SymbolUrl
TestCallable::getReceiver() const
{
    return m_receiverUrl;
}

lyric_common::TypeDef
TestCallable::getReturnType() const
{
    return m_returnType;
}

tempo_utils::Result<lyric_common::TypeDef>
TestCallable::invoke(
    lyric_assembler::BlockHandle *block,
    lyric_assembler::AbstractCallsiteReifier &reifier,
    lyric_assembler::CodeFragment *fragment)
{
    return tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kUnimplemented);
}

tempo_utils::Result<lyric_common::TypeDef>
TestCallable::invokeCtor(
    lyric_assembler::BlockHandle *block,
    lyric_assembler::AbstractCallsiteReifier &reifier,
    lyric_assembler::CodeFragment *fragment,
    tu_uint8 flags)
{
    return tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kUnimplemented);
}

tempo_utils::Result<lyric_common::TypeDef>
TestCallable::invokeNew(
    lyric_assembler::BlockHandle *block,
    lyric_assembler::AbstractCallsiteReifier &reifier,
    lyric_assembler::CodeFragment *fragment,
    tu_uint8 flags)
{
    return tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kUnimplemented);
}
