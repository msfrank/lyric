
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/link_walker.h>
#include <lyric_object/namespace_walker.h>
#include <lyric_object/symbol_walker.h>
#include <lyric_object/type_walker.h>

lyric_object::NamespaceWalker::NamespaceWalker()
    : m_namespaceOffset(INVALID_ADDRESS_U32)
{
}

lyric_object::NamespaceWalker::NamespaceWalker(
    std::shared_ptr<const internal::ObjectReader> reader,
    tu_uint32 namespaceOffset)
    : m_reader(reader),
      m_namespaceOffset(namespaceOffset)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_object::NamespaceWalker::NamespaceWalker(const NamespaceWalker &other)
    : m_reader(other.m_reader),
      m_namespaceOffset(other.m_namespaceOffset)
{
}

bool
lyric_object::NamespaceWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_namespaceOffset < m_reader->numNamespaces();
}

lyric_common::SymbolPath
lyric_object::NamespaceWalker::getSymbolPath() const
{
    if (!isValid())
        return {};
    auto *namespaceDescriptor = m_reader->getNamespace(m_namespaceOffset);
    if (namespaceDescriptor == nullptr)
        return {};
    if (namespaceDescriptor->fqsn() == nullptr)
        return {};
    return lyric_common::SymbolPath::fromString(namespaceDescriptor->fqsn()->str());
}

bool
lyric_object::NamespaceWalker::isDeclOnly() const
{
    if (!isValid())
        return false;
    auto *namespaceDescriptor = m_reader->getNamespace(m_namespaceOffset);
    if (namespaceDescriptor == nullptr)
        return false;
    return bool(namespaceDescriptor->flags() & lyo1::NamespaceFlags::DeclOnly);
}

lyric_object::AccessType
lyric_object::NamespaceWalker::getAccess() const
{
    if (!isValid())
        return AccessType::Invalid;
    auto *namespaceDescriptor = m_reader->getNamespace(m_namespaceOffset);
    if (namespaceDescriptor == nullptr)
        return AccessType::Invalid;

    if (bool(namespaceDescriptor->flags() & lyo1::NamespaceFlags::GlobalVisibility))
        return AccessType::Public;
    if (bool(namespaceDescriptor->flags() & lyo1::NamespaceFlags::InheritVisibility))
        return AccessType::Protected;
    return AccessType::Private;
}

bool
lyric_object::NamespaceWalker::hasSuperNamespace() const
{
    if (!isValid())
        return false;
    auto *namespaceDescriptor = m_reader->getNamespace(m_namespaceOffset);
    if (namespaceDescriptor == nullptr)
        return false;
    return IS_VALID(namespaceDescriptor->super_namespace());
}

lyric_object::AddressType
lyric_object::NamespaceWalker::superNamespaceAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *namespaceDescriptor = m_reader->getNamespace(m_namespaceOffset);
    if (namespaceDescriptor == nullptr)
        return AddressType::Invalid;
    return GET_ADDRESS_TYPE(namespaceDescriptor->super_namespace());
}

lyric_object::NamespaceWalker
lyric_object::NamespaceWalker::getNearSuperNamespace() const
{
    if (!hasSuperNamespace())
        return {};
    auto *namespaceDescriptor = m_reader->getNamespace(m_namespaceOffset);
    if (namespaceDescriptor == nullptr)
        return {};
    return NamespaceWalker(m_reader, GET_DESCRIPTOR_OFFSET(namespaceDescriptor->super_namespace()));
}

lyric_object::LinkWalker
lyric_object::NamespaceWalker::getFarSuperNamespace() const
{
    if (!hasSuperNamespace())
        return {};
    auto *namespaceDescriptor = m_reader->getNamespace(m_namespaceOffset);
    if (namespaceDescriptor == nullptr)
        return {};
    return LinkWalker(m_reader, GET_LINK_OFFSET(namespaceDescriptor->super_namespace()));
}

tu_uint32
lyric_object::NamespaceWalker::numBindings() const
{
    if (!isValid())
        return 0;
    auto *namespaceDescriptor = m_reader->getNamespace(m_namespaceOffset);
    if (namespaceDescriptor == nullptr)
        return 0;
    if (namespaceDescriptor->bindings() == nullptr)
        return 0;
    return namespaceDescriptor->bindings()->size();
}

lyric_object::SymbolWalker
lyric_object::NamespaceWalker::getBinding(tu_uint32 index) const
{
    if (!isValid())
        return {};
    auto *namespaceDescriptor = m_reader->getNamespace(m_namespaceOffset);
    if (namespaceDescriptor == nullptr)
        return {};
    if (namespaceDescriptor->bindings() == nullptr)
        return {};
    if (namespaceDescriptor->bindings()->size() <= index)
        return {};
    auto *binding = namespaceDescriptor->bindings()->Get(index);
    auto *symbol = m_reader->findSymbol(binding->binding_type(), binding->binding_descriptor());
    return SymbolWalker(m_reader, (void *) symbol);
}

tu_uint32
lyric_object::NamespaceWalker::getDescriptorOffset() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    return m_namespaceOffset;
}
