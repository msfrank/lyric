
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/link_walker.h>
#include <lyric_object/rest_parameter_walker.h>

lyric_object::RestParameterWalker::RestParameterWalker()
    : m_parameter(nullptr)
{
}

lyric_object::RestParameterWalker::RestParameterWalker(
    std::shared_ptr<const internal::ObjectReader> reader,
    void *parameter,
    void *names)
    : m_reader(reader),
      m_parameter(parameter),
      m_names(names)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_parameter != nullptr);
    TU_ASSERT (m_names != nullptr);
}

lyric_object::RestParameterWalker::RestParameterWalker(const RestParameterWalker &other)
    : m_reader(other.m_reader),
      m_parameter(other.m_parameter),
      m_names(other.m_names)
{
}

bool
lyric_object::RestParameterWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_parameter && m_names;
}

lyric_object::Parameter
lyric_object::RestParameterWalker::getParameter() const
{
    Parameter p;
    p.index = -1;
    p.name = getParameterName();
    p.placement = getPlacement();
    p.isVariable = isVariable();
    p.typeDef = getParameterType().getTypeDef();
    return p;
}

lyric_object::PlacementType
lyric_object::RestParameterWalker::getPlacement() const
{
    return isValid()? PlacementType::Rest : PlacementType::Invalid;
}

bool
lyric_object::RestParameterWalker::isVariable() const
{
    if (!isValid())
        return false;
    auto *parameter = static_cast<const lyo1::Parameter *>(m_parameter);
    return bool(parameter->flags() & lyo1::ParameterFlags::Var);
}

std::string
lyric_object::RestParameterWalker::getParameterName() const
{
    if (!isValid())
        return {};
    auto *parameter = static_cast<const lyo1::Parameter *>(m_parameter);
    auto *names = static_cast<const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *>(m_names);
    if (names->size() <= parameter->name_offset())
        return {};
    return std::string(names->Get(parameter->name_offset())->str());
}

lyric_object::TypeWalker
lyric_object::RestParameterWalker::getParameterType() const
{
    if (!isValid())
        return {};
    auto *parameter = static_cast<const lyo1::Parameter *>(m_parameter);
    return TypeWalker(m_reader, parameter->parameter_type());
}