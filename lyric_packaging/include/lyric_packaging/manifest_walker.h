#ifndef LYRIC_PACKAGING_MANIFEST_WALKER_H
#define LYRIC_PACKAGING_MANIFEST_WALKER_H

#include <filesystem>

#include <tempo_utils/integer_types.h>

#include "entry_path.h"
#include "entry_walker.h"
#include "package_types.h"

namespace lyric_packaging {

    class ManifestWalker {

    public:
        ManifestWalker();
        ManifestWalker(const ManifestWalker &other);

        bool isValid() const;

        EntryWalker getRoot() const;

        bool hasEntry(const EntryPath &entryPath) const;
        EntryWalker getEntry(tu_uint32 index) const;
        EntryWalker getEntry(const EntryPath &entryPath) const;
        tu_uint32 numEntries() const;

    private:
        std::shared_ptr<const internal::ManifestReader> m_reader;

        ManifestWalker(std::shared_ptr<const internal::ManifestReader> reader);
        friend class LyricManifest;
    };
}

#endif // LYRIC_PACKAGING_MANIFEST_WALKER_H