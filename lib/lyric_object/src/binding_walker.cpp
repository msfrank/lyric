
#include <lyric_object/binding_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/symbol_walker.h>
#include <lyric_object/template_walker.h>
#include <lyric_object/type_walker.h>

lyric_object::BindingWalker::BindingWalker()
    : m_bindingOffset(INVALID_ADDRESS_U32)
{
}

lyric_object::BindingWalker::BindingWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 bindingOffset)
    : m_reader(reader),
      m_bindingOffset(bindingOffset)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_object::BindingWalker::BindingWalker(const BindingWalker &other)
    : m_reader(other.m_reader),
      m_bindingOffset(other.m_bindingOffset)
{
}

bool
lyric_object::BindingWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_bindingOffset < m_reader->numBindings();
}

lyric_common::SymbolPath
lyric_object::BindingWalker::getSymbolPath() const
{
    if (!isValid())
        return {};
    auto *bindingDescriptor = m_reader->getBinding(m_bindingOffset);
    if (bindingDescriptor == nullptr)
        return {};
    if (bindingDescriptor->fqsn() == nullptr)
        return {};
    return lyric_common::SymbolPath::fromString(bindingDescriptor->fqsn()->str());
}

lyric_object::AccessType
lyric_object::BindingWalker::getAccess() const
{
    if (!isValid())
        return AccessType::Invalid;
    auto *bindingDescriptor = m_reader->getBinding(m_bindingOffset);
    if (bindingDescriptor == nullptr)
        return AccessType::Invalid;

    if (bool(bindingDescriptor->flags() & lyo1::BindingFlags::Hidden))
        return AccessType::Hidden;
    return AccessType::Public;
}

lyric_object::TypeWalker
lyric_object::BindingWalker::getBindingType() const
{
    if (!isValid())
        return {};
    auto *bindingDescriptor = m_reader->getBinding(m_bindingOffset);
    if (bindingDescriptor == nullptr)
        return {};
    if (bindingDescriptor->binding_type() == INVALID_ADDRESS_U32)
        return {};
    return TypeWalker(m_reader, bindingDescriptor->binding_type());
}

bool
lyric_object::BindingWalker::hasTemplate() const
{
    if (!isValid())
        return false;
    auto *bindingDescriptor = m_reader->getBinding(m_bindingOffset);
    if (bindingDescriptor == nullptr)
        return false;
    return bindingDescriptor->binding_template() != INVALID_ADDRESS_U32;
}

lyric_object::TemplateWalker
lyric_object::BindingWalker::getTemplate() const
{
    if (!isValid())
        return {};
    auto *bindingDescriptor = m_reader->getBinding(m_bindingOffset);
    if (bindingDescriptor == nullptr)
        return {};
    return TemplateWalker(m_reader, bindingDescriptor->binding_template());
}

lyric_object::TypeWalker
lyric_object::BindingWalker::getTargetType() const
{
    if (!isValid())
        return {};
    auto *bindingDescriptor = m_reader->getBinding(m_bindingOffset);
    if (bindingDescriptor == nullptr)
        return {};
    if (bindingDescriptor->binding_type() == INVALID_ADDRESS_U32)
        return {};
    return TypeWalker(m_reader, bindingDescriptor->target_type());
}

tu_uint32
lyric_object::BindingWalker::getDescriptorOffset() const
{
    return m_bindingOffset;
}