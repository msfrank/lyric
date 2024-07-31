#ifndef LYRIC_PARSER_STATEFUL_ATTR_H
#define LYRIC_PARSER_STATEFUL_ATTR_H

#include <tempo_utils/attr.h>

namespace lyric_parser {

    // forward declarations
    class ArchetypeNode;
    class ArchetypeState;
    class NodeWalker;

    class StatefulAttr :
        public tempo_utils::AttrValidator,
        public tempo_utils::StatefulParsingSerde<NodeWalker, std::shared_ptr<const internal::ArchetypeReader>>,
        public tempo_utils::StatefulParsingSerde<ArchetypeNode *, ArchetypeState>,
        public tempo_utils::StatefulWritingSerde<ArchetypeNode *, ArchetypeState>
    {
    public:
        explicit StatefulAttr(const tempo_utils::ComparableResource *resource)
            : tempo_utils::AttrValidator(resource) {};
    };
}

#endif // LYRIC_PARSER_STATEFUL_ATTR_H
