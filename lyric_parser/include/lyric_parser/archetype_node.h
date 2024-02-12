#ifndef LYRIC_PARSER_ARCHETYPE_NODE_H
#define LYRIC_PARSER_ARCHETYPE_NODE_H

#include <antlr4-common.h>

#include <lyric_parser/parse_result.h>
#include <tempo_utils/integer_types.h>

#include "archetype_state.h"

namespace lyric_parser {

    class ArchetypeNode {

    public:
        ArchetypeNode(
            tu_uint32 nsOffset,
            tu_uint32 typeOffset,
            antlr4::Token *token,
            NodeAddress address,
            ArchetypeState *state);
        ArchetypeNode(
            tu_uint32 nsOffset,
            tu_uint32 typeOffset,
            tu_uint32 lineNr,
            tu_uint32 columnNr,
            tu_uint32 fileOffset,
            tu_uint32 textSpan,
            NodeAddress address,
            ArchetypeState *state);

        tu_uint32 getNsOffset() const;
        tu_uint32 getTypeOffset() const;
        tu_uint32 getFileOffset() const;
        tu_uint32 getLineNumber() const;
        tu_uint32 getColumnNumber() const;
        tu_uint32 getTextSpan() const;

        NodeAddress getAddress() const;

        bool hasAttr(const AttrId &attrId) const;
        AttrAddress getAttr(const AttrId &attrId) const;
        void putAttr(ArchetypeAttr *attr);
        absl::flat_hash_map<AttrId,AttrAddress>::const_iterator attrsBegin() const;
        absl::flat_hash_map<AttrId,AttrAddress>::const_iterator attrsEnd() const;
        int numAttrs() const;

        void prependChild(ArchetypeNode *child);
        void appendChild(ArchetypeNode *child);
        NodeAddress getChild(int index);
        std::vector<NodeAddress>::const_iterator childrenBegin() const;
        std::vector<NodeAddress>::const_iterator childrenEnd() const;
        int numChildren() const;

    private:
        tu_uint32 m_nsOffset;
        tu_uint32 m_typeOffset;
        tu_uint32 m_lineNr;
        tu_uint32 m_columnNr;
        tu_uint32 m_fileOffset;
        tu_uint32 m_textSpan;
        NodeAddress m_address;
        ArchetypeState *m_state;
        absl::flat_hash_map<AttrId,AttrAddress> m_attrs;
        std::vector<NodeAddress> m_children;

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

        template<class NsType, class IdType>
        void checkClassOrThrow(const tempo_utils::SchemaClass<NsType,IdType> &schemaClass) const
        {
            if (!isClass(schemaClass)) {
                auto status = m_state->logAndContinue(ParseCondition::kParseInvariant,
                    tempo_tracing::LogSeverity::kError,
                    "expected {} node", schemaClass.getName());
                throw ParseException(status);
            }
        }
    };
}

#endif // LYRIC_PARSER_ARCHETYPE_NODE_H