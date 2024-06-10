
#include <lyric_object/call_walker.h>
#include <lyric_object/call_parameter_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/link_walker.h>

lyric_object::CallParameterWalker::CallParameterWalker()
    : m_callDescriptor(nullptr)
{
}

lyric_object::CallParameterWalker::CallParameterWalker(
    std::shared_ptr<const internal::ObjectReader> reader,
    void *callDescriptor,
    tu_uint8 parameterOffset)
    : m_reader(reader),
      m_callDescriptor(callDescriptor),
      m_parameterOffset(parameterOffset)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_callDescriptor != nullptr);
}

lyric_object::CallParameterWalker::CallParameterWalker(const CallParameterWalker &other)
    : m_reader(other.m_reader),
      m_callDescriptor(other.m_callDescriptor),
      m_parameterOffset(other.m_parameterOffset)
{
}

bool
lyric_object::CallParameterWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_callDescriptor;
}

lyric_object::Parameter
lyric_object::CallParameterWalker::getParameter() const
{
    Parameter p;
    p.index = m_parameterOffset;
    p.name = getParameterName();
    p.label = getParameterLabel();
    p.placement = getPlacement();
    p.isVariable = isVariable();
    p.typeDef = getParameterType().getTypeDef();
    return p;
}

lyric_object::PlacementType
lyric_object::CallParameterWalker::getPlacement() const
{
    if (!isValid())
        return PlacementType::Invalid;
    auto *callDescriptor = static_cast<const lyo1::CallDescriptor *>(m_callDescriptor);
    if (!callDescriptor->parameters() || callDescriptor->parameters()->size() <= m_parameterOffset)
        return PlacementType::Invalid;
    auto *parameter = callDescriptor->parameters()->Get(m_parameterOffset);

    if (bool(parameter->flags() & lyo1::ParameterFlags::Ctx))
        return PlacementType::Ctx;
    if (bool(parameter->flags() & lyo1::ParameterFlags::Rest))
        return PlacementType::Rest;

    bool hasInit = hasInitializer();
    if (bool(parameter->flags() & lyo1::ParameterFlags::Named)) {
        return hasInit? PlacementType::NamedOpt : PlacementType::Named;
    }
    return hasInit? PlacementType::ListOpt : PlacementType::List;
}

bool
lyric_object::CallParameterWalker::isVariable() const
{
    if (!isValid())
        return false;
    auto *callDescriptor = static_cast<const lyo1::CallDescriptor *>(m_callDescriptor);
    if (!callDescriptor->parameters() || callDescriptor->parameters()->size() <= m_parameterOffset)
        return false;
    auto *parameter = callDescriptor->parameters()->Get(m_parameterOffset);
    return bool(parameter->flags() & lyo1::ParameterFlags::Var);
}

std::string
lyric_object::CallParameterWalker::getParameterName() const
{
    if (!isValid())
        return {};
    auto *callDescriptor = static_cast<const lyo1::CallDescriptor *>(m_callDescriptor);
    if (!callDescriptor->parameters() || callDescriptor->parameters()->size() <= m_parameterOffset)
        return {};
    auto *parameter = callDescriptor->parameters()->Get(m_parameterOffset);

    if (!callDescriptor->names())
        return {};
    if (callDescriptor->names()->size() <= parameter->name_offset())
        return {};
    return callDescriptor->names()->Get(parameter->name_offset())->str();
}

std::string
lyric_object::CallParameterWalker::getParameterLabel() const
{
    if (!isValid())
        return {};
    auto *callDescriptor = static_cast<const lyo1::CallDescriptor *>(m_callDescriptor);
    if (!callDescriptor->parameters() || callDescriptor->parameters()->size() <= m_parameterOffset)
        return {};
    auto *parameter = callDescriptor->parameters()->Get(m_parameterOffset);

    if (!callDescriptor->names())
        return {};
    if (callDescriptor->names()->size() <= parameter->label_offset())
        return {};
    return callDescriptor->names()->Get(parameter->label_offset())->str();
}

lyric_object::TypeWalker
lyric_object::CallParameterWalker::getParameterType() const
{
    if (!isValid())
        return {};
    auto *callDescriptor = static_cast<const lyo1::CallDescriptor *>(m_callDescriptor);
    if (!callDescriptor->parameters() || callDescriptor->parameters()->size() <= m_parameterOffset)
        return {};
    auto *parameter = callDescriptor->parameters()->Get(m_parameterOffset);
    return TypeWalker(m_reader, parameter->parameter_type());
}

bool
lyric_object::CallParameterWalker::hasInitializer() const
{
    if (!isValid())
        return false;
    auto *callDescriptor = static_cast<const lyo1::CallDescriptor *>(m_callDescriptor);
    if (!callDescriptor->parameters() || callDescriptor->parameters()->size() <= m_parameterOffset)
        return false;
    auto *parameter = callDescriptor->parameters()->Get(m_parameterOffset);
    return parameter->initializer_call() != INVALID_ADDRESS_U32;
}

lyric_object::AddressType
lyric_object::CallParameterWalker::initializerAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *callDescriptor = static_cast<const lyo1::CallDescriptor *>(m_callDescriptor);
    if (!callDescriptor->parameters() || callDescriptor->parameters()->size() <= m_parameterOffset)
        return AddressType::Invalid;
    auto *parameter = callDescriptor->parameters()->Get(m_parameterOffset);
    return GET_ADDRESS_TYPE(parameter->initializer_call());
}

lyric_object::CallWalker
lyric_object::CallParameterWalker::getNearInitializer() const
{
    if (!isValid())
        return {};
    auto *callDescriptor = static_cast<const lyo1::CallDescriptor *>(m_callDescriptor);
    if (!callDescriptor->parameters() || callDescriptor->parameters()->size() <= m_parameterOffset)
        return {};
    auto *parameter = callDescriptor->parameters()->Get(m_parameterOffset);

    return CallWalker(m_reader, GET_DESCRIPTOR_OFFSET(parameter->initializer_call()));
}

lyric_object::LinkWalker
lyric_object::CallParameterWalker::getFarInitializer() const
{
    if (!isValid())
        return {};
    auto *callDescriptor = static_cast<const lyo1::CallDescriptor *>(m_callDescriptor);
    if (!callDescriptor->parameters() || callDescriptor->parameters()->size() <= m_parameterOffset)
        return {};
    auto *parameter = callDescriptor->parameters()->Get(m_parameterOffset);
    return LinkWalker(m_reader, GET_LINK_OFFSET(parameter->initializer_call()));
}
