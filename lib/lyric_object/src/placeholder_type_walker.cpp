
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/internal/type_utils.h>
#include <lyric_object/link_walker.h>
#include <lyric_object/placeholder_type_walker.h>
#include <lyric_object/template_walker.h>
#include <lyric_object/type_walker.h>

lyric_object::PlaceholderTypeWalker::PlaceholderTypeWalker()
    : m_placeholderAssignable(nullptr)
{
}

lyric_object::PlaceholderTypeWalker::PlaceholderTypeWalker(
    std::shared_ptr<const internal::ObjectReader> reader,
     void *placeholderAssignable)
    : m_reader(reader),
      m_placeholderAssignable(placeholderAssignable)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_placeholderAssignable != nullptr);
}

lyric_object::PlaceholderTypeWalker::PlaceholderTypeWalker(const PlaceholderTypeWalker &other)
    : m_reader(other.m_reader),
      m_placeholderAssignable(other.m_placeholderAssignable)
{
}

bool
lyric_object::PlaceholderTypeWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_placeholderAssignable;
}

lyric_common::TypeDef
lyric_object::PlaceholderTypeWalker::getTypeDef() const
{
    if (!isValid())
        return {};
    auto *placeholderAssignable = static_cast<const lyo1::PlaceholderAssignable *>(m_placeholderAssignable);

    TemplateWalker templateWalker(m_reader, placeholderAssignable->placeholder_template());
    if (!templateWalker.isValid())
        return {};

    auto templatePath = templateWalker.getSymbolPath();
    if (!templatePath.isValid())
        return {};
    lyric_common::SymbolUrl templateUrl(templatePath);

    std::vector<lyric_common::TypeDef> parameters;
    if (placeholderAssignable->placeholder_parameters()) {
        for (const auto &index : *placeholderAssignable->placeholder_parameters()) {
            TypeWalker parameterWalker(m_reader, index);
            if (!parameterWalker.isValid())
                return {};
            auto parameter = parameterWalker.getTypeDef();
            if (!parameter.isValid())
                return {};
            parameters.push_back(parameter);
        }
    }

    auto result = lyric_common::TypeDef::forPlaceholder(
        placeholderAssignable->placeholder_index(), templateUrl, parameters);
    return result.orElse({});
}

lyric_object::AddressType
lyric_object::PlaceholderTypeWalker::placeholderTemplateAddressType() const
{
    if (!isValid())
        return {};
    auto *placeholderAssignable = static_cast<const lyo1::PlaceholderAssignable *>(m_placeholderAssignable);
    return GET_ADDRESS_TYPE(placeholderAssignable->placeholder_template());
}

lyric_object::TemplateWalker
lyric_object::PlaceholderTypeWalker::getNearPlaceholderTemplate() const
{
    if (!isValid())
        return {};
    auto *placeholderAssignable = static_cast<const lyo1::PlaceholderAssignable *>(m_placeholderAssignable);
    return TemplateWalker(m_reader, GET_DESCRIPTOR_OFFSET(placeholderAssignable->placeholder_template()));
}

lyric_object::LinkWalker
lyric_object::PlaceholderTypeWalker::getFarPlaceholderTemplate() const
{
    if (!isValid())
        return {};
    auto *placeholderAssignable = static_cast<const lyo1::PlaceholderAssignable *>(m_placeholderAssignable);
    return LinkWalker(m_reader, GET_LINK_OFFSET(placeholderAssignable->placeholder_template()));
}

tu_uint8
lyric_object::PlaceholderTypeWalker::getPlaceholderIndex() const
{
    if (!isValid())
        return {};
    auto *placeholderAssignable = static_cast<const lyo1::PlaceholderAssignable *>(m_placeholderAssignable);
    return placeholderAssignable->placeholder_index();
}

std::vector<lyric_object::TypeWalker>
lyric_object::PlaceholderTypeWalker::getParameters() const
{
    if (!isValid())
        return {};
    auto *placeholderAssignable = static_cast<const lyo1::PlaceholderAssignable *>(m_placeholderAssignable);
    std::vector<TypeWalker> parameters;
    if (placeholderAssignable->placeholder_parameters()) {
        for (tu_uint32 p : *placeholderAssignable->placeholder_parameters()) {
            parameters.push_back(TypeWalker(m_reader, p));
        }
    }
    return parameters;
}
