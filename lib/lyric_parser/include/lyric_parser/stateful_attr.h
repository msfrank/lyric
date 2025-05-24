#ifndef LYRIC_PARSER_STATEFUL_ATTR_H
#define LYRIC_PARSER_STATEFUL_ATTR_H

#include <tempo_schema/attr_serde.h>
#include <tempo_schema/stateful_serde.h>

namespace lyric_parser {

    // forward declarations
    class ArchetypeNode;
    class ArchetypeState;
    class NodeWalker;

    class StatefulAttr :
        public tempo_schema::AttrValidator,
        public tempo_schema::StatefulParsingSerde<NodeWalker, std::shared_ptr<const internal::ArchetypeReader>>,
        public tempo_schema::StatefulParsingSerde<ArchetypeNode *, ArchetypeState>,
        public tempo_schema::StatefulWritingSerde<ArchetypeNode *, ArchetypeState>
    {
    public:
        explicit StatefulAttr(const tempo_schema::ComparableResource *resource)
            : tempo_schema::AttrValidator(resource) {};
    };
}

#endif // LYRIC_PARSER_STATEFUL_ATTR_H
