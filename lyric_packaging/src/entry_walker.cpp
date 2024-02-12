
#include <lyric_packaging/entry_walker.h>
#include <lyric_packaging/generated/manifest.h>
#include <lyric_packaging/internal/manifest_reader.h>

lyric_packaging::EntryWalker::EntryWalker()
    : m_index(kInvalidOffsetU32)
{
}

lyric_packaging::EntryWalker::EntryWalker(std::shared_ptr<const internal::ManifestReader> reader, tu_uint32 index)
    : m_reader(reader),
      m_index(index)
{
}

lyric_packaging::EntryWalker::EntryWalker(const EntryWalker &other)
    : m_reader(other.m_reader),
      m_index(other.m_index)
{
}

bool
lyric_packaging::EntryWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_index < m_reader->numEntries();
}

lyric_packaging::EntryType
lyric_packaging::EntryWalker::getEntryType() const
{
    auto *entry = m_reader->getEntry(m_index);
    if (entry == nullptr)
        return EntryType::Invalid;
    switch (entry->entry_type()) {
        case lpm1::EntryType::File:
            return EntryType::File;
        case lpm1::EntryType::Directory:
            return EntryType::Directory;
        case lpm1::EntryType::Link:
            return EntryType::Link;
        case lpm1::EntryType::Package:
            return EntryType::Package;
        default:
            return EntryType::Invalid;
    }
}

std::filesystem::path
lyric_packaging::EntryWalker::getPath() const
{
    auto *entry = m_reader->getEntry(m_index);
    if (entry == nullptr)
        return {};
    if (entry->path() == nullptr)
        return {};
    return std::filesystem::path(entry->path()->c_str(), std::filesystem::path::generic_format);
}

tu_uint64
lyric_packaging::EntryWalker::getFileOffset() const
{
    auto *entry = m_reader->getEntry(m_index);
    if (entry == nullptr)
        return {};
    return entry->entry_offset();
}

tu_uint64
lyric_packaging::EntryWalker::getFileSize() const
{
    auto *entry = m_reader->getEntry(m_index);
    if (entry == nullptr)
        return {};
    return entry->entry_size();
}

lyric_packaging::EntryWalker
lyric_packaging::EntryWalker::getLink() const
{
    auto *entry = m_reader->getEntry(m_index);
    if (entry == nullptr)
        return {};
    return EntryWalker(m_reader, entry->entry_link());
}

static lyric_packaging::EntryWalker
resolve_link(const lyric_packaging::EntryWalker &walker, int recursionDepth)
{
    if (recursionDepth == 0)
        return {};
    if (walker.getEntryType() != lyric_packaging::EntryType::Link)
        return walker;
    return resolve_link(walker.getLink(), recursionDepth--);
}

lyric_packaging::EntryWalker
lyric_packaging::EntryWalker::resolveLink() const
{
    return resolve_link(*this, kMaximumLinkRecursion);
}

bool
lyric_packaging::EntryWalker::hasAttr(const tempo_utils::AttrKey &key) const
{
    auto index = findIndexForAttr(key);
    return index != kInvalidOffsetU32;
}

bool
lyric_packaging::EntryWalker::hasAttr(const tempo_utils::AttrValidator &validator) const
{
    return hasAttr(validator.getKey());
}

tu_uint32
lyric_packaging::EntryWalker::findIndexForAttr(const tempo_utils::AttrKey &key) const
{
    if (!isValid())
        return kInvalidOffsetU32;
    auto *entry = m_reader->getEntry(m_index);
    TU_ASSERT (entry != nullptr);
    auto *attrs = entry->entry_attrs();
    if (attrs == nullptr)    // entry has no attrs
        return kInvalidOffsetU32;
    for (const auto attrIndex : *attrs) {
        auto *attr = m_reader->getAttr(attrIndex);
        TU_ASSERT (attr != nullptr);
        auto *ns = m_reader->getNamespace(attr->attr_ns());
        TU_ASSERT (ns != nullptr);
        auto *nsUrl = ns->ns_url();
        if (nsUrl == nullptr)
            continue;
        if (std::string_view(key.ns) == nsUrl->string_view() && key.id == attr->attr_id())
            return attrIndex;
    }
    return kInvalidOffsetU32;
}

int
lyric_packaging::EntryWalker::numAttrs() const
{
    if (!isValid())
        return 0;
    auto *entry = m_reader->getEntry(m_index);
    TU_ASSERT (entry != nullptr);
    auto *attrs = entry->entry_attrs();
    if (attrs == nullptr)    // entry has no attrs
        return 0;
    return attrs->size();
}

lyric_packaging::EntryWalker
lyric_packaging::EntryWalker::getChild(tu_uint32 index) const
{
    if (!isValid())
        return {};
    auto *entry = m_reader->getEntry(m_index);
    TU_ASSERT (entry != nullptr);
    auto *children = entry->entry_children();
    if (children == nullptr)    // span has no children
        return {};
    if (children->size() <= index)
        return {};
    return EntryWalker(m_reader, children->Get(index));
}

lyric_packaging::EntryWalker
lyric_packaging::EntryWalker::getChild(std::string_view name) const
{
    if (!isValid())
        return {};
    auto *entry = m_reader->getEntry(m_index);
    TU_ASSERT (entry != nullptr);
    auto *children = entry->entry_children();
    if (children == nullptr)    // span has no children
        return {};
    for (tu_uint32 i = 0; i < children->size(); i++) {
        EntryWalker child(m_reader, children->Get(i));
        auto path = child.getPath();
        if (name == path.filename().string())
            return child;
    }
    return {};
}

int
lyric_packaging::EntryWalker::numChildren() const
{
    if (!isValid())
        return 0;
    auto *entry = m_reader->getEntry(m_index);
    TU_ASSERT (entry != nullptr);
    auto *children = entry->entry_children();
    if (children == nullptr)    // span has no children
        return 0;
    return children->size();
}
