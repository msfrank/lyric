
#include <lyric_object/action_walker.h>
#include <lyric_object/call_walker.h>
#include <lyric_object/extension_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/link_walker.h>

lyric_object::ExtensionWalker::ExtensionWalker()
    : m_implDescriptor(nullptr)
{
}

lyric_object::ExtensionWalker::ExtensionWalker(
    std::shared_ptr<const internal::ObjectReader> reader,
    void *implDescriptor,
    tu_uint8 extensionOffset)
    : m_reader(reader),
      m_implDescriptor(implDescriptor),
      m_extensionOffset(extensionOffset)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_implDescriptor != nullptr);
}

lyric_object::ExtensionWalker::ExtensionWalker(const ExtensionWalker &other)
    : m_reader(other.m_reader),
      m_implDescriptor(other.m_implDescriptor),
      m_extensionOffset(other.m_extensionOffset)
{
}

bool
lyric_object::ExtensionWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_implDescriptor;
}

//std::string
//lyric_object::ExtensionWalker::getName() const
//{
//    if (!isValid())
//        return {};
//    auto *implDescriptor = static_cast<const lyo1::ImplDescriptor *>(m_implDescriptor);
//    if (implDescriptor->extensions() == nullptr)
//        return {};
//    if (implDescriptor->extensions()->size() <= m_extensionOffset)
//        return {};
//    auto *extension = implDescriptor->extensions()->Get(m_extensionOffset);
//    if (implDescriptor->names() == nullptr)
//        return {};
//    if (implDescriptor->names()->size() <= extension->name_offset())
//        return {};
//    return implDescriptor->names()->Get(extension->name_offset())->str();
//}

lyric_object::AddressType
lyric_object::ExtensionWalker::actionAddressType() const
{
    if (!isValid())
        return {};
    auto *implDescriptor = static_cast<const lyo1::ImplDescriptor *>(m_implDescriptor);
    if (implDescriptor->extensions() == nullptr)
        return {};
    if (implDescriptor->extensions()->size() <= m_extensionOffset)
        return {};
    auto *extension = implDescriptor->extensions()->Get(m_extensionOffset);
    return GET_ADDRESS_TYPE(extension->extension_action());
}

lyric_object::ActionWalker
lyric_object::ExtensionWalker::getNearAction() const
{
    if (!isValid())
        return {};
    auto *implDescriptor = static_cast<const lyo1::ImplDescriptor *>(m_implDescriptor);
    if (implDescriptor->extensions() == nullptr)
        return {};
    if (implDescriptor->extensions()->size() <= m_extensionOffset)
        return {};
    auto *extension = implDescriptor->extensions()->Get(m_extensionOffset);
    return ActionWalker(m_reader, GET_DESCRIPTOR_OFFSET(extension->extension_action()));
}

lyric_object::LinkWalker
lyric_object::ExtensionWalker::getFarAction() const
{
    if (!isValid())
        return {};
    auto *implDescriptor = static_cast<const lyo1::ImplDescriptor *>(m_implDescriptor);
    if (implDescriptor->extensions() == nullptr)
        return {};
    if (implDescriptor->extensions()->size() <= m_extensionOffset)
        return {};
    auto *extension = implDescriptor->extensions()->Get(m_extensionOffset);
    return LinkWalker(m_reader, GET_LINK_OFFSET(extension->extension_action()));
}

lyric_object::AddressType
lyric_object::ExtensionWalker::callAddressType() const
{
    if (!isValid())
        return {};
    auto *implDescriptor = static_cast<const lyo1::ImplDescriptor *>(m_implDescriptor);
    if (implDescriptor->extensions() == nullptr)
        return {};
    if (implDescriptor->extensions()->size() <= m_extensionOffset)
        return {};
    auto *extension = implDescriptor->extensions()->Get(m_extensionOffset);
    return GET_ADDRESS_TYPE(extension->extension_call());
}

lyric_object::CallWalker
lyric_object::ExtensionWalker::getNearCall() const
{
    if (!isValid())
        return {};
    auto *implDescriptor = static_cast<const lyo1::ImplDescriptor *>(m_implDescriptor);
    if (implDescriptor->extensions() == nullptr)
        return {};
    if (implDescriptor->extensions()->size() <= m_extensionOffset)
        return {};
    auto *extension = implDescriptor->extensions()->Get(m_extensionOffset);
    return CallWalker(m_reader, GET_DESCRIPTOR_OFFSET(extension->extension_call()));
}

lyric_object::LinkWalker
lyric_object::ExtensionWalker::getFarCall() const
{
    if (!isValid())
        return {};
    auto *implDescriptor = static_cast<const lyo1::ImplDescriptor *>(m_implDescriptor);
    if (implDescriptor->extensions() == nullptr)
        return {};
    if (implDescriptor->extensions()->size() <= m_extensionOffset)
        return {};
    auto *extension = implDescriptor->extensions()->Get(m_extensionOffset);
    return LinkWalker(m_reader, GET_LINK_OFFSET(extension->extension_call()));
}