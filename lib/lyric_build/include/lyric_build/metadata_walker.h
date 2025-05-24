#ifndef LYRIC_BUILD_METADATA_WALKER_H
#define LYRIC_BUILD_METADATA_WALKER_H

#include <filesystem>

#include <lyric_build/build_types.h>
#include <lyric_build/metadata_attr_parser.h>
#include <tempo_schema/attr_serde.h>
#include <tempo_schema/schema_result.h>
#include <tempo_utils/integer_types.h>

namespace lyric_build {

    class MetadataWalker {

    public:
        MetadataWalker();
        MetadataWalker(const MetadataWalker &other);

        bool isValid() const;

        bool hasAttr(const tempo_schema::AttrKey &key) const;
        bool hasAttr(const tempo_schema::AttrValidator &validator) const;
        int numAttrs() const;

    private:
        std::shared_ptr<const internal::MetadataReader> m_reader;

        MetadataWalker(std::shared_ptr<const internal::MetadataReader> reader);
        friend class LyricMetadata;

        tu_uint32 findIndexForAttr(const tempo_schema::AttrKey &key) const;

    public:
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
        parseAttr(const AttrType &attr, SerdeType &value) const {
            auto index = findIndexForAttr(attr.getKey());
            if (index == METADATA_INVALID_OFFSET_U32)
                return tempo_schema::SchemaStatus::forCondition(
                    tempo_schema::SchemaCondition::kMissingValue, "missing attr in metadata");
            MetadataAttrParser parser(m_reader);
            return attr.parseAttr(index, &parser, value);
        }
    };
}

#endif // LYRIC_BUILD_METADATA_WALKER_H