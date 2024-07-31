#ifndef LYRIC_PARSER_ATTR_VALUE_H
#define LYRIC_PARSER_ATTR_VALUE_H

#include <tempo_utils/attr.h>

namespace lyric_parser {

    // forward declarations
    class ArchetypeNode;

    class AttrValue {
    public:
        AttrValue();
        explicit AttrValue(tempo_utils::AttrValue literal);
        explicit AttrValue(ArchetypeNode *node);

        bool isValid() const;
        bool isLiteral() const;
        bool isNode() const;
        tempo_utils::AttrValue getLiteral() const;
        ArchetypeNode *getNode() const;

    private:
        tempo_utils::AttrValue m_literal;
        ArchetypeNode *m_node;
    };
}

#endif // LYRIC_PARSER_ATTR_VALUE_H
