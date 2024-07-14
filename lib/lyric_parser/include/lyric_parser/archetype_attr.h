#ifndef LYRIC_PARSER_ARCHETYPE_ATTR_H
#define LYRIC_PARSER_ARCHETYPE_ATTR_H

#include <lyric_parser/parser_types.h>
#include <tempo_utils/attr.h>

namespace lyric_parser {

    // forward declarations
    class ArchetypeId;
    class ArchetypeNamespace;
    class ArchetypeNode;
    class ArchetypeState;

    class AttrId {

    public:
        AttrId();
        AttrId(ArchetypeNamespace *attrNamespace, tu_uint32 type);
        AttrId(const AttrId &other);

        bool isValid() const;
        ArchetypeNamespace *getNamespace() const;
        tu_uint32 getType() const;

        bool operator==(const AttrId &other) const;

        template <typename H>
        friend H AbslHashValue(H h, const AttrId &id) {
            if (id.m_namespace != nullptr)
                return H::combine(std::move(h), id.getIdOffset(), id.m_type);
            return H::combine(std::move(h), 0);
        }

    private:
        ArchetypeNamespace *m_namespace;
        tu_uint32 m_type;
        tu_uint32 getIdOffset() const;
    };

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