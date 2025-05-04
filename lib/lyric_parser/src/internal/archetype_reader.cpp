
#include <flatbuffers/idl.h>

#include <lyric_parser/generated/archetype_schema.h>
#include <lyric_parser/internal/archetype_reader.h>
#include <tempo_utils/log_stream.h>

#include "lyric_parser/parser_types.h"

lyric_parser::internal::ArchetypeReader::ArchetypeReader(std::span<const tu_uint8> bytes)
    : m_bytes(bytes)
{
    m_archetype = lyi1::GetArchetype(m_bytes.data());
}

bool
lyric_parser::internal::ArchetypeReader::isValid() const
{
    return m_archetype != nullptr;
}

lyi1::ArchetypeVersion
lyric_parser::internal::ArchetypeReader::getABI() const
{
    if (m_archetype == nullptr)
        return lyi1::ArchetypeVersion::Unknown;
    return m_archetype->abi();
}

const lyi1::NamespaceDescriptor *
lyric_parser::internal::ArchetypeReader::getNamespace(tu_uint32 index) const
{
    if (m_archetype == nullptr)
        return nullptr;
    if (m_archetype->namespaces() && index < m_archetype->namespaces()->size())
        return m_archetype->namespaces()->Get(index);
    return nullptr;
}

tu_uint32
lyric_parser::internal::ArchetypeReader::numNamespaces() const
{
    if (m_archetype == nullptr)
        return 0;
    return m_archetype->namespaces()? m_archetype->namespaces()->size() : 0;
}

const lyi1::NodeDescriptor *
lyric_parser::internal::ArchetypeReader::getNode(tu_uint32 index) const
{
    if (m_archetype == nullptr)
        return nullptr;
    if (m_archetype->nodes() && index < m_archetype->nodes()->size())
        return m_archetype->nodes()->Get(index);
    return nullptr;
}

tu_uint32
lyric_parser::internal::ArchetypeReader::numNodes() const
{
    if (m_archetype == nullptr)
        return 0;
    return m_archetype->nodes()? m_archetype->nodes()->size() : 0;
}

const lyi1::AttrDescriptor *
lyric_parser::internal::ArchetypeReader::getAttr(tu_uint32 index) const
{
    if (m_archetype == nullptr)
        return nullptr;
    if (m_archetype->attrs() && index < m_archetype->attrs()->size())
        return m_archetype->attrs()->Get(index);
    return nullptr;
}

tu_uint32
lyric_parser::internal::ArchetypeReader::numAttrs() const
{
    if (m_archetype == nullptr)
        return 0;
    return m_archetype->attrs()? m_archetype->attrs()->size() : 0;
}

tu_uint32
lyric_parser::internal::ArchetypeReader::getPragma(tu_uint32 index) const
{
    if (m_archetype == nullptr)
        return INVALID_ADDRESS_U32;
    if (m_archetype->pragmas() && index < m_archetype->pragmas()->size())
        return m_archetype->pragmas()->Get(index);
    return INVALID_ADDRESS_U32;
}

tu_uint32
lyric_parser::internal::ArchetypeReader::numPragmas() const
{
    if (m_archetype == nullptr)
        return 0;
    return m_archetype->pragmas()? m_archetype->pragmas()->size() : 0;
}

tu_uint32
lyric_parser::internal::ArchetypeReader::getRoot() const
{
    if (m_archetype == nullptr)
        return 0;
    return m_archetype->root();
}

std::span<const tu_uint8>
lyric_parser::internal::ArchetypeReader::bytesView() const
{
    return m_bytes;
}

std::string
lyric_parser::internal::ArchetypeReader::dumpJson() const
{
    flatbuffers::Parser parser;
    parser.Parse((const char *) lyric_parser::schema::archetype::data);
    parser.opts.strict_json = true;

    std::string jsonData;
    auto *err = GenText(parser, m_bytes.data(), &jsonData);
    TU_ASSERT (err == nullptr);
    return jsonData;
}
