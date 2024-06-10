
#include <lyric_object/action_parameter_walker.h>
#include <lyric_object/call_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/link_walker.h>

inline const lyo1::Parameter *
cast_to_parameter(void *parameter)
{
    return static_cast<const lyo1::Parameter *>(parameter);
}

lyric_object::ActionParameterWalker::ActionParameterWalker()
    : m_parameter(nullptr),
      m_parameterOffset(0),
      m_placement(PlacementType::Invalid)
{
}

lyric_object::ActionParameterWalker::ActionParameterWalker(
    std::shared_ptr<const internal::ObjectReader> reader,
    void *parameter,
    tu_uint8 parameterOffset,
    PlacementType placement)
    : m_reader(reader),
      m_parameter(parameter),
      m_parameterOffset(parameterOffset),
      m_placement(placement)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_parameter != nullptr);
    TU_ASSERT (m_placement != PlacementType::Invalid);
}

lyric_object::ActionParameterWalker::ActionParameterWalker(const ActionParameterWalker &other)
    : m_reader(other.m_reader),
      m_parameter(other.m_parameter),
      m_parameterOffset(other.m_parameterOffset),
      m_placement(other.m_placement)
{
}

bool
lyric_object::ActionParameterWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_parameter;
}

lyric_object::Parameter
lyric_object::ActionParameterWalker::getParameter() const
{
    Parameter p;
    p.index = m_parameterOffset;
    p.name = getParameterName();
    p.placement = getPlacement();
    p.isVariable = isVariable();
    p.typeDef = getParameterType().getTypeDef();
    return p;
}

lyric_object::PlacementType
lyric_object::ActionParameterWalker::getPlacement() const
{
    if (!isValid())
        return PlacementType::Invalid;
    return m_placement;
}

bool
lyric_object::ActionParameterWalker::isVariable() const
{
    if (!isValid())
        return {};
    auto *parameter = cast_to_parameter(m_parameter);

    return bool(parameter->flags() & lyo1::ParameterFlags::Var);
}

std::string
lyric_object::ActionParameterWalker::getParameterName() const
{
    if (!isValid())
        return {};
    auto *parameter = cast_to_parameter(m_parameter);
    if (parameter->parameter_name() == nullptr)
        return {};
    return parameter->parameter_name()->str();
}

lyric_object::TypeWalker
lyric_object::ActionParameterWalker::getParameterType() const
{
    if (!isValid())
        return {};
    auto *parameter = cast_to_parameter(m_parameter);
    return TypeWalker(m_reader, parameter->parameter_type());
}

bool
lyric_object::ActionParameterWalker::hasInitializer() const
{
    if (!isValid())
        return false;
    auto *parameter = cast_to_parameter(m_parameter);
    return parameter->initializer_call() != INVALID_ADDRESS_U32;
}

lyric_object::AddressType
lyric_object::ActionParameterWalker::initializerAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *parameter = cast_to_parameter(m_parameter);
    return GET_ADDRESS_TYPE(parameter->initializer_call());
}

lyric_object::CallWalker
lyric_object::ActionParameterWalker::getNearInitializer() const
{
    if (!isValid())
        return {};
    auto *parameter = cast_to_parameter(m_parameter);
    return CallWalker(m_reader, GET_DESCRIPTOR_OFFSET(parameter->initializer_call()));
}

lyric_object::LinkWalker
lyric_object::ActionParameterWalker::getFarInitializer() const
{
    if (!isValid())
        return {};
    auto *parameter = cast_to_parameter(m_parameter);
    return LinkWalker(m_reader, GET_LINK_OFFSET(parameter->initializer_call()));
}

tu_uint8
lyric_object::ActionParameterWalker::getParameterOffset() const
{
    return m_parameterOffset;
}
