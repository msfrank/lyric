
#include <lyric_optimizer/directive_chain.h>

lyric_optimizer::DirectiveChain::DirectiveChain()
{
}

lyric_optimizer::DirectiveChain::DirectiveChain(std::shared_ptr<AbstractDirective> head)
{
    m_chain.push_front(head);
}

bool
lyric_optimizer::DirectiveChain::isValid() const
{
    return !m_chain.empty();
}

std::shared_ptr<lyric_optimizer::AbstractDirective>
lyric_optimizer::DirectiveChain::resolveDirective() const
{
    if (m_chain.empty())
        return {};
    return m_chain.front();
}

void
lyric_optimizer::DirectiveChain::forwardDirective(std::shared_ptr<AbstractDirective> directive)
{
    m_chain.push_front(directive);
}