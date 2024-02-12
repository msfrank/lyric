#ifndef LYRIC_PACKAGING_MANIFEST_NAMESPACE_H
#define LYRIC_PACKAGING_MANIFEST_NAMESPACE_H

#include <lyric_build/build_types.h>
#include <lyric_build/metadata_state.h>
#include <tempo_utils/url.h>

namespace lyric_build {

    class MetadataNamespace {

    public:
        MetadataNamespace(
            const tempo_utils::Url &nsUrl,
            NamespaceAddress address,
            MetadataState *state);

        tempo_utils::Url getNsUrl() const;
        NamespaceAddress getAddress() const;

    private:
        tempo_utils::Url m_nsUrl;
        NamespaceAddress m_address;
        MetadataState *m_state;
    };
}

#endif // LYRIC_PACKAGING_MANIFEST_NAMESPACE_H