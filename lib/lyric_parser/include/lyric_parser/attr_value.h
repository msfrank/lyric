#ifndef LYRIC_PARSER_ATTR_VALUE_H
#define LYRIC_PARSER_ATTR_VALUE_H

#include <tempo_schema/attr.h>

namespace lyric_parser {

    // forward declarations
    class ArchetypeNode;

    class AttrValue {
    public:
        AttrValue();
        explicit AttrValue(tempo_schema::AttrValue literal);
        explicit AttrValue(ArchetypeNode *node);

        bool isValid() const;
        bool isLiteral() const;
        bool isNode() const;
        tempo_schema::AttrValue getLiteral() const;
        ArchetypeNode *getNode() const;

    private:
        tempo_schema::AttrValue m_literal;
        ArchetypeNode *m_node;
    };
}

#endif // LYRIC_PARSER_ATTR_VALUE_H
