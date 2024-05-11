
#include <flatbuffers/idl.h>

#include <lyric_packaging/generated/manifest_schema.h>
#include <lyric_packaging/internal/manifest_reader.h>
#include <tempo_utils/log_stream.h>

lyric_packaging::internal::ManifestReader::ManifestReader(std::span<const tu_uint8> bytes)
    : m_bytes(bytes)
{
    m_manifest = lpm1::GetManifest(m_bytes.data());
}

bool
lyric_packaging::internal::ManifestReader::isValid() const
{
    return m_manifest != nullptr;
}

lpm1::ManifestVersion
lyric_packaging::internal::ManifestReader::getABI() const
{
    if (m_manifest == nullptr)
        return lpm1::ManifestVersion::Unknown;
    return m_manifest->abi();
}

const lpm1::NamespaceDescriptor *
lyric_packaging::internal::ManifestReader::getNamespace(uint32_t index) const
{
    if (m_manifest == nullptr)
        return nullptr;
    if (m_manifest->namespaces() && index < m_manifest->namespaces()->size())
        return m_manifest->namespaces()->Get(index);
    return nullptr;
}

uint32_t
lyric_packaging::internal::ManifestReader::numNamespaces() const
{
    if (m_manifest == nullptr)
        return 0;
    return m_manifest->namespaces()? m_manifest->namespaces()->size() : 0;
}

const lpm1::EntryDescriptor *
lyric_packaging::internal::ManifestReader::getEntry(uint32_t index) const
{
    if (m_manifest == nullptr)
        return nullptr;
    if (m_manifest->entries() && index < m_manifest->entries()->size())
        return m_manifest->entries()->Get(index);
    return nullptr;
}

uint32_t
lyric_packaging::internal::ManifestReader::numEntries() const
{
    if (m_manifest == nullptr)
        return 0;
    return m_manifest->entries()? m_manifest->entries()->size() : 0;
}

const lpm1::AttrDescriptor *
lyric_packaging::internal::ManifestReader::getAttr(uint32_t index) const
{
    if (m_manifest == nullptr)
        return nullptr;
    if (m_manifest->attrs() && index < m_manifest->attrs()->size())
        return m_manifest->attrs()->Get(index);
    return nullptr;
}

uint32_t
lyric_packaging::internal::ManifestReader::numAttrs() const
{
    if (m_manifest == nullptr)
        return 0;
    return m_manifest->attrs()? m_manifest->attrs()->size() : 0;
}

const lpm1::PathDescriptor *
lyric_packaging::internal::ManifestReader::getPath(uint32_t index) const
{
    if (m_manifest == nullptr)
        return nullptr;
    if (m_manifest->paths() && index < m_manifest->paths()->size())
        return m_manifest->paths()->Get(index);
    return nullptr;
}

const lpm1::PathDescriptor *
lyric_packaging::internal::ManifestReader::findPath(std::string_view path) const
{
    if (m_manifest == nullptr)
        return nullptr;
    std::string fullPath(path);
    return m_manifest->paths() ? m_manifest->paths()->LookupByKey(fullPath.c_str()) : nullptr;
}

uint32_t
lyric_packaging::internal::ManifestReader::numPaths() const
{
    if (m_manifest == nullptr)
        return 0;
    return m_manifest->paths()? m_manifest->paths()->size() : 0;
}

std::span<const tu_uint8>
lyric_packaging::internal::ManifestReader::bytesView() const
{
    return m_bytes;
}

std::string
lyric_packaging::internal::ManifestReader::dumpJson() const
{
    flatbuffers::Parser parser;
    parser.Parse((const char *) lyric_packaging::schema::manifest::data);
    parser.opts.strict_json = true;

    std::string jsonData;
    auto *err = GenText(parser, m_bytes.data(), &jsonData);
    TU_ASSERT (err == nullptr);
    return jsonData;
}
