#ifndef LYRIC_BUILD_INTERNAL_METADATA_READER_H
#define LYRIC_BUILD_INTERNAL_METADATA_READER_H

#include <span>

#include <lyric_build/generated/metadata.h>
#include <tempo_utils/integer_types.h>

namespace lyric_build::internal {

    class MetadataReader {

    public:
        MetadataReader(std::span<const tu_uint8> bytes);

        bool isValid() const;

        lbm1::MetadataVersion getABI() const;

        const lbm1::NamespaceDescriptor *getNamespace(uint32_t index) const;
        uint32_t numNamespaces() const;

        const lbm1::AttrDescriptor *getAttr(uint32_t index) const;
        uint32_t numAttrs() const;

        std::span<const tu_uint8> bytesView() const;

        std::string dumpJson() const;

    private:
        std::span<const tu_uint8> m_bytes;
        const lbm1::Metadata *m_metadata;
    };
}

#endif // LYRIC_BUILD_INTERNAL_METADATA_READER_H
