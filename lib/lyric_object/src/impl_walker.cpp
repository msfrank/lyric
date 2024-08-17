
#include <lyric_object/concept_walker.h>
#include <lyric_object/extension_walker.h>
#include <lyric_object/field_walker.h>
#include <lyric_object/impl_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/link_walker.h>
#include <lyric_object/symbol_walker.h>
#include <lyric_object/type_walker.h>

lyric_object::ImplWalker::ImplWalker()
    : m_implOffset(INVALID_ADDRESS_U32)
{
}

lyric_object::ImplWalker::ImplWalker(
    std::shared_ptr<const internal::ObjectReader> reader,
    tu_uint32 implOffset)
    : m_reader(reader),
      m_implOffset(implOffset)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_object::ImplWalker::ImplWalker(const ImplWalker &other)
    : m_reader(other.m_reader),
      m_implOffset(other.m_implOffset)
{
}

bool
lyric_object::ImplWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_implOffset < m_reader->numImpls();
}

bool
lyric_object::ImplWalker::isDeclOnly() const
{
    if (!isValid())
        return false;
    auto *implDescriptor = m_reader->getImpl(m_implOffset);
    if (implDescriptor == nullptr)
        return false;
    return bool(implDescriptor->flags() & lyo1::ImplFlags::DeclOnly);
}

lyric_object::TypeWalker
lyric_object::ImplWalker::getImplType() const
{
    if (!isValid())
        return {};
    auto *implDescriptor = m_reader->getImpl(m_implOffset);
    if (implDescriptor == nullptr)
        return {};
    return TypeWalker(m_reader, implDescriptor->impl_type());
}

lyric_object::AddressType
lyric_object::ImplWalker::implConceptAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *implDescriptor = m_reader->getImpl(m_implOffset);
    if (implDescriptor == nullptr)
        return AddressType::Invalid;
    return GET_ADDRESS_TYPE(implDescriptor->impl_concept());
}

lyric_object::ConceptWalker
lyric_object::ImplWalker::getNearImplConcept() const
{
    if (!isValid())
        return {};
    auto *implDescriptor = m_reader->getImpl(m_implOffset);
    if (implDescriptor == nullptr)
        return {};
    return ConceptWalker(m_reader, GET_DESCRIPTOR_OFFSET(implDescriptor->impl_concept()));
}

lyric_object::LinkWalker
lyric_object::ImplWalker::getFarImplConcept() const
{
    if (!isValid())
        return {};
    auto *implDescriptor = m_reader->getImpl(m_implOffset);
    if (implDescriptor == nullptr)
        return {};
    return LinkWalker(m_reader, GET_LINK_OFFSET(implDescriptor->impl_concept()));
}

lyric_object::SymbolWalker
lyric_object::ImplWalker::getReceiver() const
{
    if (!isValid())
        return {};
    auto *implDescriptor = m_reader->getImpl(m_implOffset);
    if (implDescriptor == nullptr)
        return {};
    lyo1::DescriptorSection section;
    switch (implDescriptor->receiver_section()) {
        case lyo1::TypeSection::Class:
            section = lyo1::DescriptorSection::Class;
            break;
        case lyo1::TypeSection::Concept:
            section = lyo1::DescriptorSection::Concept;
            break;
        case lyo1::TypeSection::Enum:
            section = lyo1::DescriptorSection::Enum;
            break;
        case lyo1::TypeSection::Existential:
            section = lyo1::DescriptorSection::Existential;
            break;
        case lyo1::TypeSection::Instance:
            section = lyo1::DescriptorSection::Instance;
            break;
        case lyo1::TypeSection::Struct:
            section = lyo1::DescriptorSection::Struct;
            break;
        default:
            return {};
    }
    tu_uint32 index = implDescriptor->receiver_descriptor();
    auto *symbol = m_reader->findSymbol(section, index);
    return SymbolWalker(m_reader, (void *) symbol);
}

lyric_object::ExtensionWalker
lyric_object::ImplWalker::getExtension(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *implDescriptor = m_reader->getImpl(m_implOffset);
    if (implDescriptor == nullptr)
        return {};
    if (implDescriptor->extensions() == nullptr)
        return {};
    if (implDescriptor->extensions()->size() <= index)
        return {};
    return ExtensionWalker(m_reader, (void *) implDescriptor, index);
}

tu_uint8
lyric_object::ImplWalker::numExtensions() const
{
    if (!isValid())
        return {};
    auto *implDescriptor = m_reader->getImpl(m_implOffset);
    if (implDescriptor == nullptr)
        return {};
    if (implDescriptor->extensions() == nullptr)
        return {};
    return implDescriptor->extensions()->size();
}

tu_uint32
lyric_object::ImplWalker::getDescriptorOffset() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    return m_implOffset;
}
