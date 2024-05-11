#ifndef LYRIC_PACKAGING_MANIFEST_NAMESPACE_H
#define LYRIC_PACKAGING_MANIFEST_NAMESPACE_H

#include <tempo_utils/url.h>

#include "manifest_state.h"
#include "package_types.h"

namespace lyric_packaging {

    class ManifestNamespace {

    public:
        ManifestNamespace(
            const tempo_utils::Url &nsUrl,
            NamespaceAddress address,
            ManifestState *state);

        tempo_utils::Url getNsUrl() const;
        NamespaceAddress getAddress() const;

    private:
        tempo_utils::Url m_nsUrl;
        NamespaceAddress m_address;
        ManifestState *m_state;
    };
}

#endif // LYRIC_PACKAGING_MANIFEST_NAMESPACE_H