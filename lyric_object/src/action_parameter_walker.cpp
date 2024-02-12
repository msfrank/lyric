
#include <lyric_object/action_parameter_walker.h>
#include <lyric_object/call_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/link_walker.h>

lyric_object::ActionParameterWalker::ActionParameterWalker()
    : m_actionDescriptor(nullptr)
{
}

lyric_object::ActionParameterWalker::ActionParameterWalker(
    std::shared_ptr<const internal::ObjectReader> reader,
    void *callDescriptor,
    tu_uint8 parameterOffset)
    : m_reader(reader),
      m_actionDescriptor(callDescriptor),
      m_parameterOffset(parameterOffset)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_actionDescriptor != nullptr);
}

lyric_object::ActionParameterWalker::ActionParameterWalker(const ActionParameterWalker &other)
    : m_reader(other.m_reader),
      m_actionDescriptor(other.m_actionDescriptor),
      m_parameterOffset(other.m_parameterOffset)
{
}

bool
lyric_object::ActionParameterWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_actionDescriptor;
}

lyric_object::Parameter
lyric_object::ActionParameterWalker::getParameter() const
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
lyric_object::ActionParameterWalker::getPlacement() const
{
    if (!isValid())
        return {};
    auto *actionDescriptor = static_cast<const lyo1::ActionDescriptor *>(m_actionDescriptor);
    if (!actionDescriptor->parameters() || actionDescriptor->parameters()->size() <= m_parameterOffset)
        return PlacementType::Invalid;
    auto *parameter = actionDescriptor->parameters()->Get(m_parameterOffset);

    if (bool(parameter->flags() & lyo1::ParameterFlags::Named)) {
        return hasInitializer()? PlacementType::Opt : PlacementType::Named;
    }
    if (bool(parameter->flags() & lyo1::ParameterFlags::Rest))
        return PlacementType::Rest;
    if (bool(parameter->flags() & lyo1::ParameterFlags::Ctx))
        return PlacementType::Ctx;
    return PlacementType::List;
}

bool
lyric_object::ActionParameterWalker::isVariable() const
{
    if (!isValid())
        return {};
    auto *actionDescriptor = static_cast<const lyo1::ActionDescriptor *>(m_actionDescriptor);
    if (!actionDescriptor->parameters() || actionDescriptor->parameters()->size() <= m_parameterOffset)
        return false;
    auto *parameter = actionDescriptor->parameters()->Get(m_parameterOffset);

    return bool(parameter->flags() & lyo1::ParameterFlags::Var);
}

std::string
lyric_object::ActionParameterWalker::getParameterName() const
{
    if (!isValid())
        return {};
    auto *actionDescriptor = static_cast<const lyo1::ActionDescriptor *>(m_actionDescriptor);
    if (!actionDescriptor->parameters() || actionDescriptor->parameters()->size() <= m_parameterOffset)
        return {};
    auto *parameter = actionDescriptor->parameters()->Get(m_parameterOffset);

    if (!actionDescriptor->names())
        return {};
    if (actionDescriptor->names()->size() <= parameter->name_offset())
        return {};
    return actionDescriptor->names()->Get(parameter->name_offset())->str();
}

std::string
lyric_object::ActionParameterWalker::getParameterLabel() const
{
    if (!isValid())
        return {};
    auto *actionDescriptor = static_cast<const lyo1::ActionDescriptor *>(m_actionDescriptor);
    if (!actionDescriptor->parameters() || actionDescriptor->parameters()->size() <= m_parameterOffset)
        return {};
    auto *parameter = actionDescriptor->parameters()->Get(m_parameterOffset);

    if (!actionDescriptor->names())
        return {};
    if (actionDescriptor->names()->size() <= parameter->label_offset())
        return {};
    return actionDescriptor->names()->Get(parameter->label_offset())->str();
}

lyric_object::TypeWalker
lyric_object::ActionParameterWalker::getParameterType() const
{
    if (!isValid())
        return {};
    auto *actionDescriptor = static_cast<const lyo1::ActionDescriptor *>(m_actionDescriptor);
    if (!actionDescriptor->parameters() || actionDescriptor->parameters()->size() <= m_parameterOffset)
        return {};
    auto *parameter = actionDescriptor->parameters()->Get(m_parameterOffset);
    return TypeWalker(m_reader, parameter->parameter_type());
}

bool
lyric_object::ActionParameterWalker::hasInitializer() const
{
    if (!isValid())
        return false;
    auto *actionDescriptor = static_cast<const lyo1::ActionDescriptor *>(m_actionDescriptor);
    if (!actionDescriptor->parameters() || actionDescriptor->parameters()->size() <= m_parameterOffset)
        return false;
    auto *parameter = actionDescriptor->parameters()->Get(m_parameterOffset);
    return parameter->initializer_call() != INVALID_ADDRESS_U32;
}

lyric_object::AddressType
lyric_object::ActionParameterWalker::initializerAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *actionDescriptor = static_cast<const lyo1::ActionDescriptor *>(m_actionDescriptor);
    if (!actionDescriptor->parameters() || actionDescriptor->parameters()->size() <= m_parameterOffset)
        return AddressType::Invalid;
    auto *parameter = actionDescriptor->parameters()->Get(m_parameterOffset);
    return GET_ADDRESS_TYPE(parameter->initializer_call());
}

lyric_object::CallWalker
lyric_object::ActionParameterWalker::getNearInitializer() const
{
    if (!isValid())
        return {};
    auto *actionDescriptor = static_cast<const lyo1::ActionDescriptor *>(m_actionDescriptor);
    if (!actionDescriptor->parameters() || actionDescriptor->parameters()->size() <= m_parameterOffset)
        return {};
    auto *parameter = actionDescriptor->parameters()->Get(m_parameterOffset);
    return CallWalker(m_reader, GET_DESCRIPTOR_OFFSET(parameter->initializer_call()));
}

lyric_object::LinkWalker
lyric_object::ActionParameterWalker::getFarInitializer() const
{
    if (!isValid())
        return {};
    auto *actionDescriptor = static_cast<const lyo1::ActionDescriptor *>(m_actionDescriptor);
    if (!actionDescriptor->parameters() || actionDescriptor->parameters()->size() <= m_parameterOffset)
        return {};
    auto *parameter = actionDescriptor->parameters()->Get(m_parameterOffset);
    return LinkWalker(m_reader, GET_LINK_OFFSET(parameter->initializer_call()));
}
