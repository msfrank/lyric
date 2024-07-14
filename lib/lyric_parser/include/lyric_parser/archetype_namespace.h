#ifndef LYRIC_PARSER_ARCHETYPE_NAMESPACE_H
#define LYRIC_PARSER_ARCHETYPE_NAMESPACE_H

#include <tempo_utils/url.h>

namespace lyric_parser {

    // forward declarations
    class ArchetypeId;
    class ArchetypeState;

    class ArchetypeNamespace {

    public:
        ArchetypeNamespace(
            const tempo_utils::Url &nsUrl,
            ArchetypeId *archetypeId,
            ArchetypeState *state);

        tempo_utils::Url getNsUrl() const;
        ArchetypeId *getArchetypeId() const;

    private:
        tempo_utils::Url m_nsUrl;
        ArchetypeId *m_archetypeId;
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_ARCHETYPE_NAMESPACE_H