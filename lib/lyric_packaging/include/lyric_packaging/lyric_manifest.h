#ifndef LYRIC_PACKAGING_LYRIC_MANIFEST_H
#define LYRIC_PACKAGING_LYRIC_MANIFEST_H

#include <span>

#include <lyric_packaging/manifest_walker.h>
#include <lyric_packaging/package_types.h>
#include <tempo_utils/immutable_bytes.h>

namespace lyric_packaging {

    class LyricManifest {

    public:
        LyricManifest();
        LyricManifest(std::shared_ptr<const tempo_utils::ImmutableBytes> immutableBytes);
        LyricManifest(std::span<const tu_uint8> unownedBytes);
        LyricManifest(const LyricManifest &other);

        bool isValid() const;

        ManifestVersion getABI() const;

        ManifestWalker getManifest() const;

        std::shared_ptr<const internal::ManifestReader> getReader() const;
        std::span<const tu_uint8> bytesView() const;

        std::string dumpJson() const;

        static bool verify(std::span<const tu_uint8> bytes);

    private:
        std::shared_ptr<const tempo_utils::ImmutableBytes> m_bytes;
        std::shared_ptr<const internal::ManifestReader> m_reader;
    };
}

#endif // LYRIC_PACKAGING_LYRIC_MANIFEST_H
