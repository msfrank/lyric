
#include <lyric_object/binding_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/link_walker.h>
#include <lyric_object/symbol_walker.h>

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

lyric_object::AddressType
lyric_object::BindingWalker::targetAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *bindingDescriptor = m_reader->getBinding(m_bindingOffset);
    if (bindingDescriptor == nullptr)
        return {};
    return GET_ADDRESS_TYPE(bindingDescriptor->target_address());
}

lyric_object::SymbolWalker
lyric_object::BindingWalker::getNearTarget() const
{
    if (!isValid())
        return {};
    auto *bindingDescriptor = m_reader->getBinding(m_bindingOffset);
    if (bindingDescriptor == nullptr)
        return {};
    return SymbolWalker(m_reader, GET_DESCRIPTOR_OFFSET(bindingDescriptor->target_address()));
}

lyric_object::LinkWalker
lyric_object::BindingWalker::getFarTarget() const
{    if (!isValid())
        return {};
    auto *bindingDescriptor = m_reader->getBinding(m_bindingOffset);
    if (bindingDescriptor == nullptr)
        return {};
    return LinkWalker(m_reader, GET_LINK_OFFSET(bindingDescriptor->target_address()));
}

lyric_object::AccessType
lyric_object::BindingWalker::getAccess() const
{
    if (!isValid())
        return AccessType::Invalid;
    auto *bindingDescriptor = m_reader->getBinding(m_bindingOffset);
    if (bindingDescriptor == nullptr)
        return AccessType::Invalid;

    if (bool(bindingDescriptor->flags() & lyo1::BindingFlags::GlobalVisibility))
        return AccessType::Public;
    if (bool(bindingDescriptor->flags() & lyo1::BindingFlags::InheritVisibility))
        return AccessType::Protected;
    return AccessType::Private;
}

tu_uint32
lyric_object::BindingWalker::getDescriptorOffset() const
{
    return m_bindingOffset;
}