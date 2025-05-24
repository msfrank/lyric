
#include <lyric_build/generated/metadata.h>
#include <lyric_build/internal/metadata_reader.h>
#include <lyric_build/metadata_walker.h>

lyric_build::MetadataWalker::MetadataWalker()
{
}

lyric_build::MetadataWalker::MetadataWalker(std::shared_ptr<const internal::MetadataReader> reader)
    : m_reader(reader)
{
}

lyric_build::MetadataWalker::MetadataWalker(const MetadataWalker &other)
    : m_reader(other.m_reader)
{
}

bool
lyric_build::MetadataWalker::isValid() const
{
    return m_reader && m_reader->isValid();
}

bool
lyric_build::MetadataWalker::hasAttr(const tempo_schema::AttrKey &key) const
{
    auto index = findIndexForAttr(key);
    return index != METADATA_INVALID_OFFSET_U32;
}

bool
lyric_build::MetadataWalker::hasAttr(const tempo_schema::AttrValidator &validator) const
{
    return hasAttr(validator.getKey());
}

tu_uint32
lyric_build::MetadataWalker::findIndexForAttr(const tempo_schema::AttrKey &key) const
{
    if (!isValid())
        return METADATA_INVALID_OFFSET_U32;
    for (tu_uint32 i = 0; i < m_reader->numAttrs(); i++) {
        auto *attr = m_reader->getAttr(i);
        TU_ASSERT (attr != nullptr);
        auto *ns = m_reader->getNamespace(attr->attr_ns());
        TU_ASSERT (ns != nullptr);
        auto *nsUrl = ns->ns_url();
        if (nsUrl == nullptr)
            continue;
        if (std::string_view(key.ns) == nsUrl->string_view() && key.id == attr->attr_id())
            return i;
    }
    return METADATA_INVALID_OFFSET_U32;
}

int
lyric_build::MetadataWalker::numAttrs() const
{
    if (!isValid())
        return 0;
    return m_reader->numAttrs();
}
