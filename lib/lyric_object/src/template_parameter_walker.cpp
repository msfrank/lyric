
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/template_parameter_walker.h>

lyric_object::TemplateParameterWalker::TemplateParameterWalker()
    : m_templateDescriptor(nullptr)
{
}

lyric_object::TemplateParameterWalker::TemplateParameterWalker(
    std::shared_ptr<const internal::ObjectReader> reader,
    void *templateDescriptor,
    tu_uint8 placeholderOffset)
    : m_reader(reader),
      m_templateDescriptor(templateDescriptor),
      m_placeholderOffset(placeholderOffset)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_templateDescriptor != nullptr);
}

lyric_object::TemplateParameterWalker::TemplateParameterWalker(const TemplateParameterWalker &other)
    : m_reader(other.m_reader),
      m_templateDescriptor(other.m_templateDescriptor),
      m_placeholderOffset(other.m_placeholderOffset)
{
}

bool
lyric_object::TemplateParameterWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_templateDescriptor;
}

lyric_object::TemplateParameter
lyric_object::TemplateParameterWalker::getTemplateParameter() const
{
    if (!isValid())
        return {};

    TemplateParameter tp;
    tp.index = m_placeholderOffset;
    tp.name = getPlaceholderName();
    tp.variance = getPlaceholderVariance();
    tp.bound = getConstraintBound();
    tp.typeDef = getConstraintType().getTypeDef();
    return tp;
}

std::string
lyric_object::TemplateParameterWalker::getPlaceholderName() const
{
    if (!isValid())
        return {};
    auto *templateDescriptor = static_cast<const lyo1::TemplateDescriptor *>(m_templateDescriptor);
    if (!templateDescriptor->placeholders())
        return {};
    auto *placeholder = templateDescriptor->placeholders()->Get(m_placeholderOffset);
    if (!templateDescriptor->names())
        return {};
    if (templateDescriptor->names()->size() <= placeholder->name_offset())
        return {};
    return templateDescriptor->names()->Get(placeholder->name_offset())->str();
}

lyric_object::VarianceType
lyric_object::TemplateParameterWalker::getPlaceholderVariance() const
{
    if (!isValid())
        return VarianceType::Invalid;
    auto *templateDescriptor = static_cast<const lyo1::TemplateDescriptor *>(m_templateDescriptor);
    if (!templateDescriptor->placeholders())
        return VarianceType::Invalid;
    auto *placeholder = templateDescriptor->placeholders()->Get(m_placeholderOffset);
    switch (placeholder->placeholder_variance()) {
        case lyo1::PlaceholderVariance::Invariant:
            return VarianceType::Invariant;
        case lyo1::PlaceholderVariance::Covariant:
            return VarianceType::Covariant;
        case lyo1::PlaceholderVariance::Contravariant:
            return VarianceType::Contravariant;
        default:
            return VarianceType::Invalid;
    }
}

inline const lyo1::Constraint *
get_constraint(const lyo1::TemplateDescriptor *templateDescriptor, tu_uint8 placeholderOffset) {
    if (!templateDescriptor->constraints())
        return nullptr;
    for (const auto &constraint : *templateDescriptor->constraints()) {
        if (constraint->placeholder_offset() == placeholderOffset)
            return constraint;
    }
    return nullptr;
}

bool
lyric_object::TemplateParameterWalker::hasConstraint() const
{
    if (!isValid())
        return {};
    auto *templateDescriptor = static_cast<const lyo1::TemplateDescriptor *>(m_templateDescriptor);
    return get_constraint(templateDescriptor, m_placeholderOffset) != nullptr;
}

lyric_object::TypeWalker
lyric_object::TemplateParameterWalker::getConstraintType() const
{
    if (!isValid())
        return {};
    auto *templateDescriptor = static_cast<const lyo1::TemplateDescriptor *>(m_templateDescriptor);
    auto *constraint = get_constraint(templateDescriptor, m_placeholderOffset);
    if (constraint == nullptr)
        return {};
    return TypeWalker(m_reader, constraint->constraint_type());
}

lyric_object::BoundType
lyric_object::TemplateParameterWalker::getConstraintBound() const
{
    if (!isValid())
        return {};
    auto *templateDescriptor = static_cast<const lyo1::TemplateDescriptor *>(m_templateDescriptor);
    auto *constraint = get_constraint(templateDescriptor, m_placeholderOffset);
    if (constraint == nullptr)
        return BoundType::None;
    switch (constraint->constraint_bound()) {
        case lyo1::ConstraintBound::Extends:
            return BoundType::Extends;
        case lyo1::ConstraintBound::Super:
            return BoundType::Super;
        default:
            return BoundType::Invalid;
    }
}