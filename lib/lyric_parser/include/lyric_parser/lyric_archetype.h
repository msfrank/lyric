#ifndef LYRIC_PARSER_LYRIC_ARCHETYPE_H
#define LYRIC_PARSER_LYRIC_ARCHETYPE_H

#include <span>

#include <tempo_utils/immutable_bytes.h>

#include "node_walker.h"
#include "parser_types.h"

namespace lyric_parser {

    class LyricArchetype {

    public:
        LyricArchetype();
        LyricArchetype(std::shared_ptr<const tempo_utils::ImmutableBytes> immutableBytes);
        LyricArchetype(std::span<const tu_uint8> unownedBytes);
        LyricArchetype(const LyricArchetype &other);

        bool isValid() const;

        ArchetypeVersion getABI() const;

        NodeWalker getNode(tu_uint32 index) const;
        uint32_t numNodes() const;

        NodeWalker getRoot() const;

        std::shared_ptr<const internal::ArchetypeReader> getReader() const;
        std::span<const tu_uint8> bytesView() const;

        static bool verify(std::span<const tu_uint8> bytes);

    private:
        std::shared_ptr<const tempo_utils::ImmutableBytes> m_bytes;
        std::shared_ptr<const internal::ArchetypeReader> m_reader;
    };
}

#endif // LYRIC_PARSER_LYRIC_ARCHETYPE_H
