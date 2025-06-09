#ifndef LYRIC_BUILD_METADATA_WRITER_H
#define LYRIC_BUILD_METADATA_WRITER_H

#include <lyric_build/build_result.h>
#include <lyric_build/build_types.h>
#include <lyric_build/metadata_attr_writer.h>
#include <lyric_build/metadata_state.h>

namespace lyric_build {

    struct MetadataWriterOptions {
        MetadataVersion version = MetadataVersion::Version1;
        /**
         *
         */
        LyricMetadata metadata = {};
    };

    class MetadataWriter {

    public:
        explicit MetadataWriter(const MetadataWriterOptions &options = {});

        tempo_utils::Status configure();

        tempo_utils::Result<LyricMetadata> toMetadata();

    private:
        MetadataWriterOptions m_options;
        std::unique_ptr<MetadataState> m_state;

    public:
        /**
         *
         * @tparam T
         * @param serde
         * @param value
         * @return
         */
        template <typename T>
        tempo_utils::Status putAttr(const tempo_schema::AttrSerde<T> &serde, const T &value)
        {
            if (m_state == nullptr)
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "metadata writer is not configured");
            MetadataAttrWriter writer(serde.getKey(), m_state.get());
            auto result = serde.writeAttr(&writer, value);
            if (result.isStatus())
                return result.getStatus();
            return {};
        }
    };
}

#endif // LYRIC_BUILD_METADATA_WRITER_H