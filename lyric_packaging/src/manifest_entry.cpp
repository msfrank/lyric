
#include <lyric_packaging/manifest_entry.h>

lyric_packaging::ManifestEntry::ManifestEntry(
    EntryType type,
    const EntryPath &path,
    EntryAddress address,
    ManifestState *state)
    : m_type(type),
      m_path(path),
      m_address(address),
      m_offset(kInvalidOffsetU32),
      m_size(kInvalidOffsetU32),
      m_state(state)
{
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_state != nullptr);
}

lyric_packaging::EntryType
lyric_packaging::ManifestEntry::getEntryType() const
{
    return m_type;
}

lyric_packaging::EntryPath
lyric_packaging::ManifestEntry::getEntryPath() const
{
    return m_path;
}

std::string
lyric_packaging::ManifestEntry::getEntryName() const
{
    if (m_path.isValid())
        return m_path.getFilename();
    return {};
}

lyric_packaging::EntryAddress
lyric_packaging::ManifestEntry::getAddress() const
{
    return m_address;
}

tu_uint32
lyric_packaging::ManifestEntry::getEntryOffset() const
{
    return m_offset;
}

void
lyric_packaging::ManifestEntry::setEntryOffset(tu_uint32 offset)
{
    m_offset = offset;
}

tu_uint32
lyric_packaging::ManifestEntry::getEntrySize() const
{
    return m_size;
}

void
lyric_packaging::ManifestEntry::setEntrySize(tu_uint32 size)
{
    m_size = size;
}

lyric_packaging::EntryAddress
lyric_packaging::ManifestEntry::getEntryDict() const
{
    return m_dict;
}

void
lyric_packaging::ManifestEntry::setEntryDict(EntryAddress dict)
{
    m_dict = dict;
}

lyric_packaging::EntryAddress
lyric_packaging::ManifestEntry::getEntryLink() const
{
    return m_link;
}

void
lyric_packaging::ManifestEntry::setEntryLink(EntryAddress link)
{
    m_link = link;
}

bool
lyric_packaging::ManifestEntry::hasAttr(const AttrId &attrId) const
{
    return m_attrs.contains(attrId);
}

lyric_packaging::AttrAddress
lyric_packaging::ManifestEntry::getAttr(const AttrId &attrId) const
{
    if (m_attrs.contains(attrId))
        return m_attrs.at(attrId);
    return {};
}

lyric_packaging::PackageStatus
lyric_packaging::ManifestEntry::putAttr(ManifestAttr *attr)
{
    TU_ASSERT (attr != nullptr);

    auto attrId = attr->getAttrId();
    if (m_attrs.contains(attrId)) {
        return PackageStatus::forCondition(
            PackageCondition::kPackageInvariant, "entry contains duplicate attr");
    }
    m_attrs[attrId] = attr->getAddress();
    return PackageStatus::ok();
}

absl::flat_hash_map<
    lyric_packaging::AttrId,
    lyric_packaging::AttrAddress>::const_iterator
lyric_packaging::ManifestEntry::attrsBegin() const
{
    return m_attrs.cbegin();
}

absl::flat_hash_map<
    lyric_packaging::AttrId,
    lyric_packaging::AttrAddress>::const_iterator
lyric_packaging::ManifestEntry::attrsEnd() const
{
    return m_attrs.cend();
}

int
lyric_packaging::ManifestEntry::numAttrs() const
{
    return m_attrs.size();
}

bool
lyric_packaging::ManifestEntry::hasChild(std::string_view name) const
{
    return m_children.contains(name);
}

lyric_packaging::EntryAddress
lyric_packaging::ManifestEntry::getChild(std::string_view name)
{
    if (m_children.contains(name))
        return m_children.at(name);
    return {};
}

lyric_packaging::PackageStatus
lyric_packaging::ManifestEntry::putChild(ManifestEntry *child)
{
    TU_ASSERT (child != nullptr);
    auto name = child->getEntryName();
    if (name.empty())
        return PackageStatus::forCondition(
            PackageCondition::kPackageInvariant, "invalid entry name");
    m_children[name] = child->getAddress();
    return PackageStatus::ok();
}

absl::flat_hash_map<
    std::string,
    lyric_packaging::EntryAddress>::const_iterator
lyric_packaging::ManifestEntry::childrenBegin() const
{
    return m_children.cbegin();
}

absl::flat_hash_map<
    std::string,
    lyric_packaging::EntryAddress>::const_iterator
lyric_packaging::ManifestEntry::childrenEnd() const
{
    return m_children.cend();
}

int lyric_packaging::ManifestEntry::numChildren() const
{
    return m_children.size();
}
