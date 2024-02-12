#ifndef LYRIC_PACKAGING_PACKAGE_READER_H
#define LYRIC_PACKAGING_PACKAGE_READER_H

#include <filesystem>

#include <tempo_utils/attr.h>
#include <tempo_utils/integer_types.h>
#include <tempo_utils/memory_mapped_bytes.h>

#include "lyric_manifest.h"
#include "manifest_state.h"
#include "manifest_walker.h"
#include "package_result.h"

namespace lyric_packaging {

    class PackageReader {

    public:
        static tempo_utils::Result<std::shared_ptr<PackageReader>> create(
            std::shared_ptr<const tempo_utils::ImmutableBytes> packageBytes);
        static tempo_utils::Result<std::shared_ptr<PackageReader>> open(
            const std::filesystem::path &packagePath);

        bool isValid() const;

        tu_uint8 getVersion() const;
        tu_uint8 getFlags() const;
        LyricManifest getManifest() const;

        tempo_utils::Slice getFileContents(
            const EntryPath &entryPath,
            bool followSymlinks = false) const;
        tu_uint32 getFileSize(
            const EntryPath &entryPath,
            bool followSymlinks = false) const;

    private:
        tu_uint8 m_version;
        tu_uint8 m_flags;
        LyricManifest m_manifest;
        tempo_utils::Slice m_contents;

        PackageReader(
            tu_uint8 version,
            tu_uint8 flags,
            LyricManifest manifest,
            tempo_utils::Slice contents);
    };
}

#endif // LYRIC_PACKAGING_PACKAGE_READER_H