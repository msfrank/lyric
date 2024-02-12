#ifndef LYRIC_SERDE_LYRIC_PATCHSET_H
#define LYRIC_SERDE_LYRIC_PATCHSET_H

#include <span>

#include <lyric_serde/patchset_walker.h>
#include <lyric_serde/serde_types.h>
#include <tempo_utils/immutable_bytes.h>
#include <tempo_utils/integer_types.h>

namespace lyric_serde {

    class LyricPatchset {

    public:
        LyricPatchset();
        LyricPatchset(std::shared_ptr<const tempo_utils::ImmutableBytes> immutableBytes);
        LyricPatchset(std::span<const tu_uint8> unownedBytes);
        LyricPatchset(const LyricPatchset &other);

        bool isValid() const;

        PatchsetVersion getABI() const;

        PatchsetWalker getPatchset() const;

        std::shared_ptr<const internal::PatchsetReader> getReader() const;
        std::span<const tu_uint8> bytesView() const;

        static bool verify(std::span<const tu_uint8> bytes);

    private:
        std::shared_ptr<const tempo_utils::ImmutableBytes> m_bytes;
        std::shared_ptr<const internal::PatchsetReader> m_reader;
    };
}

#endif // LYRIC_SERDE_LYRIC_PATCHSET_H
