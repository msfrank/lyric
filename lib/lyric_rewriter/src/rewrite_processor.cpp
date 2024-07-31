
#include <lyric_rewriter/rewrite_processor.h>
#include <lyric_rewriter/rewriter_result.h>

lyric_rewriter::RewriteProcessor::RewriteProcessor()
{
}

tempo_utils::Status
lyric_rewriter::RewriteProcessor::process(
    lyric_parser::ArchetypeState *state,
    std::shared_ptr<AbstractNodeVisitor> visitor)
{
    TU_ASSERT (state != nullptr);
    TU_ASSERT (visitor != nullptr);

    auto *root = state->getRoot();
    if (root == nullptr)
        return RewriterStatus::forCondition(
            RewriterCondition::kRewriterInvariant, "archetype state has no root");

    //
    m_stack.emplace_back(Visitation{-1, root, std::move(visitor), false});

    while (!isEmpty()) {
        auto &visitation = peek();
        VisitorContext ctx(state, visitation.visitor, parentNode(), visitation.index);
        if (!visitation.seen) {
            TU_RETURN_IF_NOT_OK (visitation.visitor->enter(visitation.node, ctx));
            visitation.seen = true;
            ctx.extend(m_stack);
        } else {
            TU_RETURN_IF_NOT_OK (visitation.visitor->exit(visitation.node, ctx));
            pop();
        }
    }

    return {};
}

bool
lyric_rewriter::RewriteProcessor::isEmpty()
{
    return m_stack.empty();
}

lyric_rewriter::Visitation&
lyric_rewriter::RewriteProcessor::peek()
{
    TU_ASSERT (!m_stack.empty());
    return m_stack.back();
}

lyric_parser::ArchetypeNode *
lyric_rewriter::RewriteProcessor::parentNode()
{
    if (m_stack.size() <= 1)
        return nullptr;
    auto &parent = m_stack.at(m_stack.size() - 2);
    return parent.node;
}

void
lyric_rewriter::RewriteProcessor::pop()
{
    TU_ASSERT (!m_stack.empty());
    m_stack.pop_back();
}

lyric_rewriter::VisitorContext::VisitorContext(
    lyric_parser::ArchetypeState *state,
    std::shared_ptr<AbstractNodeVisitor> curr,
    lyric_parser::ArchetypeNode *parent,
    int index)
    : m_state(state),
      m_curr(curr),
      m_parent(parent),
      m_index(index),
      m_skip(false)
{
    TU_ASSERT (m_curr != nullptr);
}

lyric_parser::ArchetypeState *
lyric_rewriter::VisitorContext::archetypeState() const
{
    return m_state;
}

int
lyric_rewriter::VisitorContext::childIndex() const
{
    return m_index;
}

lyric_parser::ArchetypeNode *
lyric_rewriter::VisitorContext::parentNode() const
{
    return m_parent;
}

bool
lyric_rewriter::VisitorContext::skipChildren() const
{
    return m_skip;
}

void
lyric_rewriter::VisitorContext::setSkipChildren(bool skip)
{
    m_skip = skip;
}

void
lyric_rewriter::VisitorContext::push(
    int childIndex,
    lyric_parser::ArchetypeNode *childNode,
    std::shared_ptr<AbstractNodeVisitor> visitor)
{
    Visitation visitation;
    visitation.index = childIndex;
    visitation.node = childNode;
    visitation.visitor = visitor? visitor : m_curr;
    visitation.seen = false;
    m_added.push_back(std::move(visitation));
}

void
lyric_rewriter::VisitorContext::extend(std::vector<Visitation> &stack)
{
    stack.insert(stack.end(), m_added.cbegin(), m_added.cend());
}
