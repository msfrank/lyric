#ifndef LYRIC_BUILD_LYRIC_METADATA_H
#define LYRIC_BUILD_LYRIC_METADATA_H

#include <span>

#include <tempo_schema/attr_serde.h>
#include <tempo_schema/schema_result.h>
#include <tempo_utils/immutable_bytes.h>

#include "build_types.h"
#include "metadata_attr_parser.h"

namespace lyric_build {

    class LyricMetadata {

    public:
        LyricMetadata();
        LyricMetadata(std::shared_ptr<const tempo_utils::ImmutableBytes> immutableBytes);
        LyricMetadata(std::span<const tu_uint8> unownedBytes);
        LyricMetadata(const LyricMetadata &other);

        bool isValid() const;

        MetadataVersion getABI() const;

        bool hasAttr(const tempo_schema::AttrKey &key) const;
        bool hasAttr(const tempo_schema::AttrValidator &validator) const;
        tempo_schema::Attr getAttr(tu_uint32 index) const;
        int numAttrs() const;

        std::shared_ptr<const internal::MetadataReader> getReader() const;
        std::span<const tu_uint8> bytesView() const;

        std::string dumpJson() const;

        static bool verify(std::span<const tu_uint8> bytes);

    private:
        std::shared_ptr<const tempo_utils::ImmutableBytes> m_bytes;
        std::shared_ptr<const internal::MetadataReader> m_reader;

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

#endif // LYRIC_BUILD_LYRIC_METADATA_H
