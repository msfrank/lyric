#ifndef LYRIC_PARSER_ARCHETYPE_ATTR_H
#define LYRIC_PARSER_ARCHETYPE_ATTR_H

#include <lyric_parser/parser_types.h>
#include <tempo_utils/attr.h>

#include "attr_id.h"
#include "attr_value.h"

namespace lyric_parser {

    // forward declarations
    class ArchetypeId;
    class ArchetypeNamespace;
    class ArchetypeNode;
    class ArchetypeState;

    class ArchetypeAttr {

    public:
        ArchetypeAttr(
            AttrId id,
            AttrValue value,
            ArchetypeId *archetypeId,
            ArchetypeState *state);

        AttrId getAttrId() const;
        AttrValue getAttrValue() const;
        ArchetypeId *getArchetypeId() const;

    private:
        AttrId m_id;
        AttrValue m_value;
        ArchetypeId *m_archetypeId;
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_ARCHETYPE_ATTR_H