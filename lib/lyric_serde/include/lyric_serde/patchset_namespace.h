#ifndef LYRIC_SERDE_PATCHSET_NAMESPACE_H
#define LYRIC_SERDE_PATCHSET_NAMESPACE_H

#include <tempo_utils/url.h>

#include "patchset_state.h"
#include "serde_types.h"

namespace lyric_serde {

    class PatchsetNamespace {

    public:
        PatchsetNamespace(
            const tempo_utils::Url &nsUrl,
            NamespaceAddress address,
            PatchsetState *state);

        tempo_utils::Url getNsUrl() const;
        NamespaceAddress getAddress() const;

    private:
        tempo_utils::Url m_nsUrl;
        NamespaceAddress m_address;
        PatchsetState *m_state;
    };
}

#endif // LYRIC_SERDE_PATCHSET_NAMESPACE_H