#ifndef LYRIC_REWRITER_PRAGMA_REWRITER_H
#define LYRIC_REWRITER_PRAGMA_REWRITER_H

#include <lyric_schema/ast_schema.h>

#include "abstract_rewrite_driver.h"

namespace lyric_rewriter {

    class PragmaRewriter {
    public:
        PragmaRewriter(
            std::shared_ptr<AbstractRewriteDriverBuilder> rewriteDriverBuilder,
            lyric_parser::ArchetypeState *state);

        tempo_utils::Status rewritePragmas();

    private:
        std::shared_ptr<AbstractRewriteDriverBuilder> m_rewriteDriverBuilder;
        lyric_parser::ArchetypeState *m_state;
    };
}

#endif // LYRIC_REWRITER_PRAGMA_REWRITER_H
