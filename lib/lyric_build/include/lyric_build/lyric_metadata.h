#ifndef LYRIC_BUILD_LYRIC_METADATA_H
#define LYRIC_BUILD_LYRIC_METADATA_H

#include <span>

#include <lyric_build/build_types.h>
#include <lyric_build/metadata_walker.h>
#include <tempo_utils/immutable_bytes.h>

namespace lyric_build {

    class LyricMetadata {

    public:
        LyricMetadata();
        LyricMetadata(std::shared_ptr<const tempo_utils::ImmutableBytes> immutableBytes);
        LyricMetadata(std::span<const tu_uint8> unownedBytes);
        LyricMetadata(const LyricMetadata &other);

        bool isValid() const;

        MetadataVersion getABI() const;
        MetadataWalker getMetadata() const;

        std::shared_ptr<const internal::MetadataReader> getReader() const;
        std::span<const tu_uint8> bytesView() const;

        std::string dumpJson() const;

        static bool verify(std::span<const tu_uint8> bytes);

    private:
        std::shared_ptr<const tempo_utils::ImmutableBytes> m_bytes;
        std::shared_ptr<const internal::MetadataReader> m_reader;
    };
}

#endif // LYRIC_BUILD_LYRIC_METADATA_H
