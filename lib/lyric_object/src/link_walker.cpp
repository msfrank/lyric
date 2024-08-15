
#include <lyric_object/import_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/link_walker.h>

lyric_object::LinkWalker::LinkWalker()
    : m_linkOffset(INVALID_ADDRESS_U32)
{
}

lyric_object::LinkWalker::LinkWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 linkOffset)
    : m_reader(reader),
      m_linkOffset(linkOffset)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_object::LinkWalker::LinkWalker(const LinkWalker &other)
    : m_reader(other.m_reader),
      m_linkOffset(other.m_linkOffset)
{
}

bool
lyric_object::LinkWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_linkOffset < m_reader->numLinks();
}

lyric_common::SymbolPath
lyric_object::LinkWalker::getSymbolPath() const
{
    if (!isValid())
        return {};
    auto *linkDescriptor = m_reader->getLink(m_linkOffset);
    if (linkDescriptor == nullptr)
        return {};
    if (linkDescriptor->fqsn() == nullptr)
        return {};
    return lyric_common::SymbolPath::fromString(linkDescriptor->fqsn()->str());
}

lyric_object::LinkageSection
lyric_object::LinkWalker::getLinkageSection() const
{
    if (!isValid())
        return LinkageSection::Invalid;
    auto *linkDescriptor = m_reader->getLink(m_linkOffset);
    if (linkDescriptor == nullptr)
        return {};
    return internal::descriptor_to_linkage_section(linkDescriptor->link_type());
}

tu_uint32
lyric_object::LinkWalker::getLinkageIndex() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    return m_linkOffset;
}

lyric_object::ImportWalker
lyric_object::LinkWalker::getLinkImport() const
{
    if (!isValid())
        return {};
    auto *linkDescriptor = m_reader->getLink(m_linkOffset);
    if (linkDescriptor == nullptr)
        return {};
    return ImportWalker(m_reader, linkDescriptor->link_import());
}

lyric_common::SymbolUrl
lyric_object::LinkWalker::getLinkUrl() const
{
    if (!isValid())
        return {};
    auto *linkDescriptor = m_reader->getLink(m_linkOffset);
    if (linkDescriptor == nullptr)
        return {};
    if (linkDescriptor->fqsn() == nullptr)
        return {};
    auto linkPath = lyric_common::SymbolPath::fromString(linkDescriptor->fqsn()->str());

    auto *importDescriptor = m_reader->getImport(linkDescriptor->link_import());
    if (importDescriptor == nullptr)
        return {};
    if (importDescriptor->import_location() == nullptr)
        return {};
    auto linkLocation = lyric_common::ModuleLocation::fromString(importDescriptor->import_location()->str());

    return lyric_common::SymbolUrl(linkLocation, linkPath);
}

tu_uint32
lyric_object::LinkWalker::getDescriptorOffset() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    return m_linkOffset;
}
