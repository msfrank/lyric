#ifndef LYRIC_PARSER_NODE_WALKER_H
#define LYRIC_PARSER_NODE_WALKER_H

#include <tempo_utils/attr.h>
#include <tempo_utils/integer_types.h>

#include "archetype_reader_attr_parser.h"
#include "parser_types.h"
#include "parse_result.h"

namespace lyric_parser {

    class NodeWalker {

    public:
        NodeWalker();
        NodeWalker(const NodeWalker &other);

        bool isValid() const;

        NodeAddress getAddress() const;
        ParseLocation getLocation() const;
        tu_uint32 getLineNumber() const;
        tu_uint32 getColumnNumber() const;
        tu_uint32 getFileOffset() const;
        tu_uint32 getTextSpan() const;

        std::string_view namespaceView() const;
        bool isNamespace(const tempo_utils::SchemaNs &schemaNs) const;

        tu_uint32 getIdValue() const;

        bool hasAttr(const tempo_utils::AttrKey &key) const;
        bool hasAttr(const tempo_utils::AttrValidator &validator) const;
        std::pair<tempo_utils::AttrKey,tempo_utils::AttrValue> getAttr(int index) const;
        int numAttrs() const;

        NodeWalker getChild(tu_uint32 index) const;
        int numChildren() const;
        NodeWalker getNodeAtOffset(tu_uint32 offset) const;

    private:
        std::shared_ptr<const internal::ArchetypeReader> m_reader;
        tu_uint32 m_index;

        NodeWalker(std::shared_ptr<const internal::ArchetypeReader> reader, tu_uint32 index);
        friend class LyricArchetype;
        friend class NodeAttr;

        bool matchesNsAndId(const char *nsUrl, tu_uint32 idValue) const;
        tu_uint32 findIndexForAttr(const tempo_utils::AttrKey &key) const;
        const char *getNsUrl() const;

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
        tempo_utils::Status
        parseId(
            const tempo_utils::SchemaVocabulary<NsType,IdType> &vocabulary,
            IdType &id) const
        {
            auto *schemaNs = vocabulary.getNs();
            if (!isNamespace(*schemaNs))
                return ParseStatus::forCondition(ParseCondition::kSyntaxError,
                    "expected node ns {}", vocabulary.getNs()->getNs());
            auto *resource = vocabulary.getResource(getIdValue());
            if (resource == nullptr)
                return ParseStatus::forCondition(ParseCondition::kSyntaxError,
                    "unknown node type {}", getIdValue());
            id = resource->getId();
            return ParseStatus::ok();
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
            auto index = findIndexForAttr(attr.getKey());
            if (index == INVALID_ADDRESS_U32)
                return ParseStatus::forCondition(ParseCondition::kParseInvariant, "missing attr in node");
            ArchetypeReaderAttrParser parser(m_reader);
            return attr.parseAttr(index, &parser, value);
        }
    };
}

#endif // LYRIC_PARSER_NODE_WALKER_H