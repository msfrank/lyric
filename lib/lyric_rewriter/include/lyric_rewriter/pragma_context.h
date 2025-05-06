#ifndef LYRIC_REWRITER_PRAGMA_CONTEXT_H
#define LYRIC_REWRITER_PRAGMA_CONTEXT_H

#include <lyric_parser/archetype_node.h>

namespace lyric_rewriter {

    class PragmaContext {
    public:
        PragmaContext();

        tempo_utils::Status appendNode(lyric_parser::ArchetypeNode *node);
        std::vector<lyric_parser::ArchetypeNode *>::const_iterator pragmasBegin() const;
        std::vector<lyric_parser::ArchetypeNode *>::const_iterator pragmasEnd() const;
        int numPragmas() const;

    private:
        std::vector<lyric_parser::ArchetypeNode *> m_pragmas;
    };
}

#endif // LYRIC_REWRITER_PRAGMA_CONTEXT_H
