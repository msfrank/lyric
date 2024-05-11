#ifndef LYRIC_BUILD_METADATA_WRITER_H
#define LYRIC_BUILD_METADATA_WRITER_H

#include <filesystem>
#include <string>

#include <lyric_build/build_types.h>
#include <lyric_build/metadata_attr_writer.h>
#include <lyric_build/metadata_result.h>
#include <lyric_build/metadata_state.h>
#include <tempo_utils/attr.h>

namespace lyric_build {

    struct MetadataWriterOptions {
        MetadataVersion version = MetadataVersion::Version1;
    };

    class MetadataWriter {

    public:
        MetadataWriter();
        MetadataWriter(const MetadataWriterOptions &options);

        tempo_utils::Result<LyricMetadata> toMetadata();

    private:
        MetadataWriterOptions m_options;
        MetadataState m_state;

    public:
        /**
         *
         * @tparam T
         * @param serde
         * @param value
         * @return
         */
        template <typename T>
        tempo_utils::Status putAttr(const tempo_utils::AttrSerde<T> &serde, const T &value)
        {
            MetadataAttrWriter writer(serde.getKey(), &m_state);
            auto result = serde.writeAttr(&writer, value);
            if (result.isStatus())
                return result.getStatus();
            return tempo_utils::AttrStatus::ok();
        }
    };
}

#endif // LYRIC_BUILD_METADATA_WRITER_H