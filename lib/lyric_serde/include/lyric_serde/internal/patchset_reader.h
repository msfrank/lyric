#ifndef LYRIC_SERDE_INTERNAL_PATCHSET_READER_H
#define LYRIC_SERDE_INTERNAL_PATCHSET_READER_H

#include <span>

#include <lyric_serde/generated/patchset.h>
#include <tempo_utils/integer_types.h>

namespace lyric_serde::internal {

    class PatchsetReader {

    public:
        PatchsetReader(std::span<const tu_uint8> bytes);

        bool isValid() const;

        lps1::PatchsetVersion getABI() const;

        const lps1::NamespaceDescriptor *getNamespace(uint32_t index) const;
        uint32_t numNamespaces() const;

        const lps1::ValueDescriptor *getValue(uint32_t index) const;
        uint32_t numValues() const;

        const lps1::ChangeDescriptor *getChange(uint32_t index) const;
        uint32_t numChanges() const;

        std::span<const tu_uint8> bytesView() const;

        std::string dumpJson() const;

    private:
        std::span<const tu_uint8> m_bytes;
        const lps1::Patchset *m_patchset;
    };
}

#endif // LYRIC_SERDE_INTERNAL_PATCHSET_READER_H
