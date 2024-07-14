#ifndef LYRIC_PARSER_INTERNAL_ARCHETYPE_WRITER_H
#define LYRIC_PARSER_INTERNAL_ARCHETYPE_WRITER_H

#include <vector>

#include <flatbuffers/flatbuffers.h>

#include <lyric_parser/generated/archetype.h>
#include <tempo_utils/integer_types.h>

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ArchetypeWriter {
    public:
        static tempo_utils::Result<LyricArchetype> createArchetype(const ArchetypeState *state);

    private:
        const ArchetypeState *m_state;
        flatbuffers::FlatBufferBuilder m_buffer;
        std::vector<tu_uint32> m_addressTable;
        std::vector<flatbuffers::Offset<lyi1::NamespaceDescriptor>> m_namespacesVector;
        std::vector<flatbuffers::Offset<lyi1::AttrDescriptor>> m_attrsVector;
        std::vector<flatbuffers::Offset<lyi1::NodeDescriptor>> m_nodesVector;

        explicit ArchetypeWriter(const ArchetypeState *state);
        tempo_utils::Result<NamespaceAddress> writeNamespace(const ArchetypeNamespace *ns);
        tempo_utils::Result<std::pair<lyi1::Value,flatbuffers::Offset<void>>> writeValue(
            const lyric_parser::AttrValue &value);
        tempo_utils::Result<AttrAddress> writeAttr(const ArchetypeAttr *attr);
        tempo_utils::Result<NodeAddress> writeNode(const ArchetypeNode *node);
        tempo_utils::Result<LyricArchetype> writeArchetype();
    };
}

#endif // LYRIC_PARSER_INTERNAL_ARCHETYPE_WRITER_H
