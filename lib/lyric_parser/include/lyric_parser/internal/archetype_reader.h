#ifndef LYRIC_PARSER_INTERNAL_ARCHETYPE_READER_H
#define LYRIC_PARSER_INTERNAL_ARCHETYPE_READER_H

#include <span>
#include <string>

#include <lyric_parser/generated/archetype.h>
#include <tempo_utils/integer_types.h>

namespace lyric_parser::internal {

    class ArchetypeReader {

    public:
        ArchetypeReader(std::span<const tu_uint8> bytes);

        bool isValid() const;

        lyi1::ArchetypeVersion getABI() const;

        const lyi1::NamespaceDescriptor *getNamespace(tu_uint32 index) const;
        tu_uint32 numNamespaces() const;

        const lyi1::NodeDescriptor *getNode(tu_uint32 index) const;
        tu_uint32 numNodes() const;

        const lyi1::AttrDescriptor *getAttr(tu_uint32 index) const;
        tu_uint32 numAttrs() const;

        tu_uint32 getPragma(tu_uint32 index) const;
        tu_uint32 numPragmas() const;

        tu_uint32 getRoot() const;

        std::span<const tu_uint8> bytesView() const;

        std::string dumpJson() const;

    private:
        std::span<const tu_uint8> m_bytes;
        const lyi1::Archetype *m_archetype;
    };
}

#endif // LYRIC_PARSER_INTERNAL_ARCHETYPE_READER_H