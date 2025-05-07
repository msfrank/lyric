
#include <lyric_rewriter/pragma_context.h>
#include <lyric_rewriter/pragma_rewriter.h>

lyric_rewriter::PragmaRewriter::PragmaRewriter(
    std::shared_ptr<AbstractRewriteDriverBuilder> rewriteDriverBuilder,
    lyric_parser::ArchetypeState *state)
    : m_rewriteDriverBuilder(rewriteDriverBuilder),
      m_state(state)
{
    TU_ASSERT (m_rewriteDriverBuilder != nullptr);
    TU_ASSERT (m_state != nullptr);
}

tempo_utils::Status
lyric_rewriter::PragmaRewriter::rewritePragmas()
{
    std::vector pragmaNodes(m_state->pragmasBegin(), m_state->pragmasEnd());
    PragmaContext ctx;

    // loop over all existing nodes in the pragmas section
    for (auto *node : pragmaNodes) {
        bool isRewritten = false;

        // if the existing node is an AST pragma then rewrite it
        if (node->isNamespace(lyric_schema::kLyricAstNs)) {
            auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());
            auto astId = resource->getId();
            if (astId == lyric_schema::LyricAstId::Pragma) {
                TU_RETURN_IF_NOT_OK (m_rewriteDriverBuilder->rewritePragma(m_state, node, ctx));
                isRewritten = true;
            }
        }

        // otherwise if the node was not rewritten then append it to the context
        if (!isRewritten) {
            ctx.appendNode(node);
        }
    }

    // rewrite the pragmas section with the nodes from the context
    m_state->clearPragmas();
    for (auto it = ctx.pragmasBegin(); it != ctx.pragmasEnd(); it++) {
        m_state->addPragma(*it);
    }

    return {};
}
