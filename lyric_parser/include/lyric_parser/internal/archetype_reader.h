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

        const lyi1::NamespaceDescriptor *getNamespace(uint32_t index) const;
        uint32_t numNamespaces() const;

        const lyi1::NodeDescriptor *getNode(uint32_t index) const;
        uint32_t numNodes() const;

        const lyi1::AttributeDescriptor *getAttr(uint32_t index) const;
        uint32_t numAttrs() const;

        std::span<const tu_uint8> bytesView() const;

        std::string dumpJson() const;

    private:
        std::span<const tu_uint8> m_bytes;
        const lyi1::Archetype *m_archetype;
    };
}

#endif // LYRIC_PARSER_INTERNAL_ARCHETYPE_READER_H