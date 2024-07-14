#ifndef LYRIC_PARSER_ARCHETYPE_NODE_H
#define LYRIC_PARSER_ARCHETYPE_NODE_H

#include <lyric_parser/parse_result.h>
#include <tempo_utils/integer_types.h>

#include "archetype_attr.h"
#include "archetype_namespace.h"

namespace lyric_parser {

    // forward declarations
    class ArchetypeId;
    class ArchetypeNamespace;
    class ArchetypeState;

    class ArchetypeNode {

    public:
        ArchetypeNode(
            ArchetypeNamespace *nodeNamespace,
            tu_uint32 typeOffset,
            const ParseLocation &location,
            ArchetypeId *archetypeId,
            ArchetypeState *state);
//        ArchetypeNode(
//            ArchetypeNamespace *nodeNamespace,
//            tu_uint32 typeOffset,
//            tu_uint32 lineNr,
//            tu_uint32 columnNr,
//            tu_uint32 fileOffset,
//            tu_uint32 textSpan,
//            ArchetypeId *archetypeId,
//            ArchetypeState *state);

        ArchetypeNamespace *getNamespace() const;
        tu_uint32 getTypeOffset() const;
        ParseLocation getLocation() const;
        ArchetypeId *getArchetypeId() const;

        bool hasAttr(const AttrId &attrId) const;
        ArchetypeAttr *getAttr(const AttrId &attrId) const;
        void putAttr(ArchetypeAttr *attr);
        absl::flat_hash_map<AttrId,ArchetypeAttr *>::const_iterator attrsBegin() const;
        absl::flat_hash_map<AttrId,ArchetypeAttr *>::const_iterator attrsEnd() const;
        int numAttrs() const;

        void prependChild(ArchetypeNode *child);
        void appendChild(ArchetypeNode *child);
        ArchetypeNode *getChild(int index) const;
        ArchetypeNode *detachChild(int index);
        std::vector<ArchetypeNode *>::const_iterator childrenBegin() const;
        std::vector<ArchetypeNode *>::const_iterator childrenEnd() const;
        int numChildren() const;

    private:
        ArchetypeNamespace *m_namespace;
        tu_uint32 m_typeOffset;
        ParseLocation m_location;
        ArchetypeId *m_archetypeId;
        ArchetypeState *m_state;
        absl::flat_hash_map<AttrId,ArchetypeAttr *> m_attrs;
        std::vector<ArchetypeNode *> m_children;

        bool matchesNsAndId(const char *nsString, tu_uint32 idValue) const;

    public:
        /**
         *
         * @tparam NsType
         * @tparam IdType
         * @param schemaClass
         * @return
         */
        template<class NsType, class IdType>
        bool isClass(const tempo_utils::SchemaClass<NsType,IdType> &schemaClass) const
        {
            auto *nsUrl = schemaClass.getNsUrl();
            auto idValue = schemaClass.getIdValue();
            return matchesNsAndId(nsUrl, idValue);
        }
    };
}

#endif // LYRIC_PARSER_ARCHETYPE_NODE_H