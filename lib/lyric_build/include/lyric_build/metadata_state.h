#ifndef LYRIC_BUILD_METADATA_STATE_H
#define LYRIC_BUILD_METADATA_STATE_H

#include <lyric_build/build_types.h>
#include <lyric_build/lyric_metadata.h>
#include <tempo_schema/attr.h>
#include <tempo_utils/url.h>

namespace lyric_build {

    // forward declarations
    class MetadataNamespace;
    class MetadataAttr;

    class MetadataState {

    public:
        MetadataState();

        tempo_utils::Status load(const LyricMetadata &metadata);

        bool hasNamespace(const tempo_utils::Url &nsUrl) const;
        MetadataNamespace *getNamespace(int index) const;
        MetadataNamespace *getNamespace(const tempo_utils::Url &nsUrl) const;
        tempo_utils::Result<MetadataNamespace *> putNamespace(const tempo_utils::Url &nsUrl);
        std::vector<MetadataNamespace *>::const_iterator namespacesBegin() const;
        std::vector<MetadataNamespace *>::const_iterator namespacesEnd() const;
        int numNamespaces() const;

        tempo_utils::Result<MetadataAttr *> appendAttr(AttrId id, const tempo_schema::AttrValue &value);
        MetadataAttr *getAttr(int index) const;
        std::vector<MetadataAttr *>::const_iterator attrsBegin() const;
        std::vector<MetadataAttr *>::const_iterator attrsEnd() const;
        int numAttrs() const;

        tempo_utils::Result<LyricMetadata> toMetadata() const;

    private:
        std::vector<MetadataNamespace *> m_metadataNamespaces;
        std::vector<MetadataAttr *> m_metadataAttrs;
        absl::flat_hash_map<tempo_utils::Url,tu_uint32> m_namespaceIndex;
    };
}

#endif // LYRIC_BUILD_METADATA_STATE_H
