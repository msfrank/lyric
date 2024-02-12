#ifndef LYRIC_PACKAGING_INTERNAL_MANIFEST_READER_H
#define LYRIC_PACKAGING_INTERNAL_MANIFEST_READER_H

#include <span>

#include <lyric_packaging/generated/manifest.h>
#include <tempo_utils/integer_types.h>

namespace lyric_packaging::internal {

    class ManifestReader {

    public:
        ManifestReader(std::span<const tu_uint8> bytes);

        bool isValid() const;

        lpm1::ManifestVersion getABI() const;

        const lpm1::NamespaceDescriptor *getNamespace(uint32_t index) const;
        uint32_t numNamespaces() const;

        const lpm1::AttrDescriptor *getAttr(uint32_t index) const;
        uint32_t numAttrs() const;

        const lpm1::EntryDescriptor *getEntry(uint32_t index) const;
        uint32_t numEntries() const;

        const lpm1::PathDescriptor *getPath(uint32_t index) const;
        const lpm1::PathDescriptor *findPath(std::string_view path) const;
        uint32_t numPaths() const;

        std::span<const tu_uint8> bytesView() const;

        std::string dumpJson() const;

    private:
        std::span<const tu_uint8> m_bytes;
        const lpm1::Manifest *m_manifest;
    };
}

#endif // LYRIC_PACKAGING_INTERNAL_MANIFEST_READER_H
