#ifndef LYRIC_PARSER_ARCHETYPE_NAMESPACE_H
#define LYRIC_PARSER_ARCHETYPE_NAMESPACE_H

#include <tempo_utils/url.h>

#include "archetype_state.h"

namespace lyric_parser {

    class ArchetypeNamespace {

    public:
        ArchetypeNamespace(
            const tempo_utils::Url &nsUrl,
            NamespaceAddress address,
            ArchetypeState *state);

        tempo_utils::Url getNsUrl() const;
        NamespaceAddress getAddress() const;

    private:
        tempo_utils::Url m_nsUrl;
        NamespaceAddress m_address;
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_ARCHETYPE_NAMESPACE_H