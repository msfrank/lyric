
#include <lyric_rewriter/fail_unknown_visitor.h>

#include "lyric_rewriter/rewriter_result.h"

lyric_rewriter::FailUnknownVisitor::FailUnknownVisitor(AbstractProcessorState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

tempo_utils::Status
lyric_rewriter::FailUnknownVisitor::enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx)
{
    return RewriterStatus::forCondition(RewriterCondition::kSyntaxError,
        "unexpected node with namespace: {} id: {}",
        node->getNamespace()->getNsUrl().toString(), node->getIdValue());
}

tempo_utils::Status
lyric_rewriter::FailUnknownVisitor::exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx)
{
    return RewriterStatus::forCondition(RewriterCondition::kSyntaxError,
        "unexpected node with namespace: {} id: {}",
        node->getNamespace()->getNsUrl().toString(), node->getIdValue());
}
