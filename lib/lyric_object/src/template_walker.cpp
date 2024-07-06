
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/template_parameter_walker.h>
#include <lyric_object/template_walker.h>

lyric_object::TemplateWalker::TemplateWalker()
    : m_templateOffset(INVALID_ADDRESS_U32)
{
}

lyric_object::TemplateWalker::TemplateWalker(
    std::shared_ptr<const internal::ObjectReader> reader,
    tu_uint32 templateOffset)
    : m_reader(reader),
      m_templateOffset(templateOffset)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_templateOffset != INVALID_ADDRESS_U32);
}

lyric_object::TemplateWalker::TemplateWalker(const TemplateWalker &other)
    : m_reader(other.m_reader),
      m_templateOffset(other.m_templateOffset)
{
}

bool
lyric_object::TemplateWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_templateOffset < m_reader->numTemplates();
}

lyric_common::SymbolPath
lyric_object::TemplateWalker::getSymbolPath() const
{
    if (!isValid())
        return {};
    auto *templateDescriptor = m_reader->getTemplate(m_templateOffset);
    if (templateDescriptor == nullptr)
        return {};
    if (templateDescriptor->fqsn() == nullptr)
        return {};
    return lyric_common::SymbolPath::fromString(templateDescriptor->fqsn()->str());
}

bool
lyric_object::TemplateWalker::hasSuperTemplate() const
{
    if (!isValid())
        return false;
    auto *templateDescriptor = m_reader->getTemplate(m_templateOffset);
    if (templateDescriptor == nullptr)
        return {};
    return templateDescriptor->super_template() != INVALID_ADDRESS_U32;
}

lyric_object::TemplateWalker
lyric_object::TemplateWalker::getSuperTemplate() const
{
    if (!hasSuperTemplate())
        return {};
    auto *templateDescriptor = m_reader->getTemplate(m_templateOffset);
    if (templateDescriptor == nullptr)
        return {};
    return TemplateWalker(m_reader, GET_DESCRIPTOR_OFFSET(templateDescriptor->super_template()));
}

lyric_object::TemplateParameterWalker
lyric_object::TemplateWalker::getTemplateParameter(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *templateDescriptor = m_reader->getTemplate(m_templateOffset);
    if (templateDescriptor == nullptr)
        return {};
    if (templateDescriptor->placeholders() == nullptr)
        return {};
    if (templateDescriptor->placeholders()->size() <= index)
        return {};
    return TemplateParameterWalker(m_reader, (void *) templateDescriptor, index);
}

tu_uint8
lyric_object::TemplateWalker::numTemplateParameters() const
{
    if (!isValid())
        return 0;
    auto *templateDescriptor = m_reader->getTemplate(m_templateOffset);
    if (templateDescriptor == nullptr)
        return 0;
    if (templateDescriptor->placeholders())
        return templateDescriptor->placeholders()->size();
    return 0;
}

tu_uint32
lyric_object::TemplateWalker::getDescriptorOffset() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    return m_templateOffset;
}
