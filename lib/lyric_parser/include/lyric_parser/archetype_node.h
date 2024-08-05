#ifndef LYRIC_PARSER_ARCHETYPE_NODE_H
#define LYRIC_PARSER_ARCHETYPE_NODE_H

#include <lyric_parser/parse_result.h>
#include <tempo_utils/integer_types.h>

#include "archetype_namespace.h"
#include "archetype_state_attr_parser.h"
#include "archetype_state_attr_writer.h"
#include "attr_id.h"
#include "attr_value.h"
#include "node_walker.h"
#include "stateful_attr.h"

namespace lyric_parser {

    // forward declarations
    class ArchetypeAttr;
    class ArchetypeId;
    class ArchetypeNamespace;
    class ArchetypeState;

    class ArchetypeNode {

    public:
        ArchetypeNode(
            ArchetypeNamespace *nodeNamespace,
            tu_uint32 idValue,
            const ParseLocation &location,
            ArchetypeId *archetypeId,
            ArchetypeState *state);

        NodeWalker getArchetypeNode() const;

        ArchetypeNamespace *getNamespace() const;
        tu_uint32 getIdValue() const;
        ParseLocation getLocation() const;
        ArchetypeId *getArchetypeId() const;

        std::string_view namespaceView() const;
        bool isNamespace(const tempo_utils::SchemaNs &schemaNs) const;

        bool hasAttr(const AttrId &attrId) const;
        bool hasAttr(const tempo_utils::AttrValidator &validator) const;
        ArchetypeAttr *getAttr(const AttrId &attrId) const;
        void putAttr(ArchetypeAttr *attr);
        absl::flat_hash_map<AttrId,ArchetypeAttr *>::const_iterator attrsBegin() const;
        absl::flat_hash_map<AttrId,ArchetypeAttr *>::const_iterator attrsEnd() const;
        int numAttrs() const;

        ArchetypeNode *getChild(int index) const;
        void prependChild(ArchetypeNode *child);
        void appendChild(ArchetypeNode *child);
        void insertChild(int index, ArchetypeNode *child);
        ArchetypeNode *replaceChild(int index, ArchetypeNode *child);
        ArchetypeNode *removeChild(int index);
        std::vector<ArchetypeNode *>::const_iterator childrenBegin() const;
        std::vector<ArchetypeNode *>::const_iterator childrenEnd() const;
        int numChildren() const;

    private:
        NodeWalker m_archetypeNode;
        ArchetypeNamespace *m_namespace;
        tu_uint32 m_idValue;
        ParseLocation m_location;
        ArchetypeId *m_archetypeId;
        ArchetypeState *m_state;
        absl::flat_hash_map<AttrId,ArchetypeAttr *> m_attrs;
        std::vector<ArchetypeNode *> m_children;

        bool matchesNsAndId(const char *nsString, tu_uint32 idValue) const;
        ArchetypeAttr *findAttr(const char *nsString, tu_uint32 idValue) const;
        tu_uint32 findAttrIndex(const char *nsString, tu_uint32 idValue) const;
        AttrValue getAttrValue(const char *nsString, tu_uint32 idValue) const;

        ArchetypeNode(
            const NodeWalker &archetypeNode,
            ArchetypeNamespace *nodeNamespace,
            tu_uint32 idValue,
            const ParseLocation &location,
            ArchetypeId *archetypeId,
            ArchetypeState *state);

        friend class ArchetypeState;

    public:
        /**
         *
         * @tparam NsType
         * @tparam IdType
         * @param schemaClass
         * @return
         */
        template<class NsType, class IdType>
        bool
        isClass(const tempo_utils::SchemaClass<NsType,IdType> &schemaClass) const
        {
            auto *nsUrl = schemaClass.getNsUrl();
            auto idValue = schemaClass.getIdValue();
            return matchesNsAndId(nsUrl, idValue);
        }

        template<class NsType, class IdType>
        tempo_utils::Status
        parseId(
            const tempo_utils::SchemaVocabulary<NsType,IdType> &vocabulary,
            IdType &id) const
        {
            auto *schemaNs = vocabulary.getNs();
            if (!isNamespace(*schemaNs))
                return ParseStatus::forCondition(ParseCondition::kSyntaxError,
                    "expected node ns {}", vocabulary.getNs()->getNs());
            auto *resource = vocabulary.getResource(m_idValue);
            if (resource == nullptr)
                return ParseStatus::forCondition(ParseCondition::kSyntaxError,
                    "unknown node id {}", m_idValue);
            id = resource->getId();
            return ParseStatus::ok();
        }

        template<class NsType, class IdType>
        ArchetypeAttr *
        getAttr(const tempo_utils::SchemaProperty<NsType,IdType> &schemaProperty) const
        {
            return findAttr(schemaProperty.getNsUrl(), schemaProperty.getIdValue());
        }

        template<class NsType, class IdType>
        AttrValue
        getAttrValue(const tempo_utils::SchemaProperty<NsType,IdType> &schemaProperty) const
        {
            return getAttrValue(schemaProperty.getNsUrl(), schemaProperty.getIdValue());
        }

        /**
         *
         * @tparam AttrType
         * @tparam SerdeType
         * @param attr
         * @param value
         * @return
         */
        template<class AttrType,
            typename SerdeType = typename AttrType::SerdeType>
        tempo_utils::Status
        parseAttr(const AttrType &attr, SerdeType &value) const
        {
            auto key = attr.getKey();
            auto index = findAttrIndex(key.ns, key.id);
            if (index == INVALID_ADDRESS_U32)
                return ParseStatus::forCondition(ParseCondition::kParseInvariant, "missing attr in node");
            ArchetypeStateAttrParser parser(m_state);
            return attr.parseAttr(index, &parser, value);
        }

        template <typename T>
        tempo_utils::Status
        putAttr(const tempo_utils::AttrSerde<T> &serde, const T &value)
        {
            ArchetypeAttr *archetypeAttr;
            TU_ASSIGN_OR_RETURN (archetypeAttr, ArchetypeStateAttrWriter::createAttr(m_state, serde, value));
            putAttr(archetypeAttr);
            return {};
        };

        template <class T>
        tempo_utils::Status
        putAttr(const StatefulAttr &attr, const T &value)
        {
            ArchetypeAttr *archetypeAttr;
            auto key = attr.getKey();
            TU_ASSIGN_OR_RETURN (archetypeAttr, ArchetypeStateAttrWriter::createAttr(m_state, key, attr, value));
            putAttr(archetypeAttr);
            return {};
        };

        template <typename T>
        void
        putAttrOrThrow(const tempo_utils::AttrSerde<T> &serde, const T &value)
        {
            TU_RAISE_IF_NOT_OK(putAttr(serde, value));
        };

        template <class T>
        void
        putAttrOrThrow(const StatefulAttr &attr, const T &value)
        {
            TU_RAISE_IF_NOT_OK(putAttr(attr, value));
        };
    };
}

#endif // LYRIC_PARSER_ARCHETYPE_NODE_H