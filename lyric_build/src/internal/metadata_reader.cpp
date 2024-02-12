
#include <flatbuffers/idl.h>

#include <lyric_build/generated/metadata.h>
#include <lyric_build/generated/metadata_schema.h>
#include <lyric_build/internal/metadata_reader.h>
#include <tempo_utils/log_stream.h>

lyric_build::internal::MetadataReader::MetadataReader(std::span<const tu_uint8> bytes)
    : m_bytes(bytes)
{
    m_metadata = lbm1::GetMetadata(m_bytes.data());
}

bool
lyric_build::internal::MetadataReader::isValid() const
{
    return m_metadata != nullptr;
}

lbm1::MetadataVersion
lyric_build::internal::MetadataReader::getABI() const
{
    if (m_metadata == nullptr)
        return lbm1::MetadataVersion::Unknown;
    return m_metadata->abi();
}

const lbm1::NamespaceDescriptor *
lyric_build::internal::MetadataReader::getNamespace(uint32_t index) const
{
    if (m_metadata == nullptr)
        return nullptr;
    if (m_metadata->namespaces() && index < m_metadata->namespaces()->size())
        return m_metadata->namespaces()->Get(index);
    return nullptr;
}

uint32_t
lyric_build::internal::MetadataReader::numNamespaces() const
{
    if (m_metadata == nullptr)
        return 0;
    return m_metadata->namespaces()? m_metadata->namespaces()->size() : 0;
}

const
lbm1::AttrDescriptor *lyric_build::internal::MetadataReader::getAttr(uint32_t index) const
{
    if (m_metadata == nullptr)
        return nullptr;
    if (m_metadata->attrs() && index < m_metadata->attrs()->size())
        return m_metadata->attrs()->Get(index);
    return nullptr;
}

uint32_t
lyric_build::internal::MetadataReader::numAttrs() const
{
    if (m_metadata == nullptr)
        return 0;
    return m_metadata->attrs()? m_metadata->attrs()->size() : 0;
}

std::span<const tu_uint8>
lyric_build::internal::MetadataReader::bytesView() const
{
    return m_bytes;
}

std::string
lyric_build::internal::MetadataReader::dumpJson() const
{
    flatbuffers::Parser parser;
    parser.Parse((const char *) lyric_build::schema::metadata::data);
    parser.opts.strict_json = true;

    std::string jsonData;
    auto *err = GenText(parser, m_bytes.data(), &jsonData);
    TU_ASSERT (err == nullptr);
    return jsonData;
}
