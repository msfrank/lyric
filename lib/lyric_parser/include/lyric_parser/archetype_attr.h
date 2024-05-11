#ifndef LYRIC_PARSER_ARCHETYPE_ATTR_H
#define LYRIC_PARSER_ARCHETYPE_ATTR_H

#include <lyric_parser/parser_types.h>
#include <tempo_utils/attr.h>

#include "archetype_state.h"

namespace lyric_parser {

    class ArchetypeAttr {

    public:
        ArchetypeAttr(
            AttrId id,
            tempo_utils::AttrValue value,
            AttrAddress address,
            ArchetypeState *state);

        AttrId getAttrId() const;
        tempo_utils::AttrValue getAttrValue() const;
        AttrAddress getAddress() const;

    private:
        AttrId m_id;
        tempo_utils::AttrValue m_value;
        AttrAddress m_address;
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_ARCHETYPE_ATTR_H