
#include <flatbuffers/idl.h>

#include <lyric_serde/generated/patchset_schema.h>
#include <lyric_serde/internal/patchset_reader.h>
#include <tempo_utils/log_stream.h>

lyric_serde::internal::PatchsetReader::PatchsetReader(std::span<const tu_uint8> bytes)
    : m_bytes(bytes)
{
    m_patchset = lps1::GetPatchset(m_bytes.data());
}

bool
lyric_serde::internal::PatchsetReader::isValid() const
{
    return m_patchset != nullptr;
}

lps1::PatchsetVersion
lyric_serde::internal::PatchsetReader::getABI() const
{
    if (m_patchset == nullptr)
        return lps1::PatchsetVersion::Unknown;
    return m_patchset->abi();
}

const lps1::NamespaceDescriptor *
lyric_serde::internal::PatchsetReader::getNamespace(uint32_t index) const
{
    if (m_patchset == nullptr)
        return nullptr;
    if (m_patchset->namespaces() && index < m_patchset->namespaces()->size())
        return m_patchset->namespaces()->Get(index);
    return nullptr;
}

uint32_t
lyric_serde::internal::PatchsetReader::numNamespaces() const
{
    if (m_patchset == nullptr)
        return 0;
    return m_patchset->namespaces()? m_patchset->namespaces()->size() : 0;
}

const
lps1::ValueDescriptor *lyric_serde::internal::PatchsetReader::getValue(uint32_t index) const
{
    if (m_patchset == nullptr)
        return nullptr;
    if (m_patchset->values() && index < m_patchset->values()->size())
        return m_patchset->values()->Get(index);
    return nullptr;
}

uint32_t
lyric_serde::internal::PatchsetReader::numValues() const
{
    if (m_patchset == nullptr)
        return 0;
    return m_patchset->values()? m_patchset->values()->size() : 0;
}

const
lps1::ChangeDescriptor *lyric_serde::internal::PatchsetReader::getChange(uint32_t index) const
{
    if (m_patchset == nullptr)
        return nullptr;
    if (m_patchset->changes() && index < m_patchset->changes()->size())
        return m_patchset->changes()->Get(index);
    return nullptr;
}

uint32_t
lyric_serde::internal::PatchsetReader::numChanges() const
{
    if (m_patchset == nullptr)
        return 0;
    return m_patchset->changes()? m_patchset->changes()->size() : 0;
}

std::span<const tu_uint8>
lyric_serde::internal::PatchsetReader::bytesView() const
{
    return m_bytes;
}

std::string
lyric_serde::internal::PatchsetReader::dumpJson() const
{
    flatbuffers::Parser parser;
    parser.Parse((const char *) lyric_serde::schema::patchset::data);
    parser.opts.strict_json = true;

    std::string jsonData;
    auto *err = GenText(parser, m_bytes.data(), &jsonData);
    TU_ASSERT (err == nullptr);
    return jsonData;
}
