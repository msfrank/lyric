#ifndef LYRIC_OPTIMIZER_DIRECTIVE_CHAIN_H
#define LYRIC_OPTIMIZER_DIRECTIVE_CHAIN_H

#include <memory>
#include <forward_list>

#include "abstract_directive.h"

namespace lyric_optimizer {

    class DirectiveChain {
    public:
        DirectiveChain();
        explicit DirectiveChain(std::shared_ptr<AbstractDirective> head);

        bool isValid() const;
        std::shared_ptr<AbstractDirective> resolveDirective() const;
        void forwardDirective(std::shared_ptr<AbstractDirective> directive);

    private:
        std::forward_list<std::shared_ptr<AbstractDirective>> m_chain;
    };
}

#endif // LYRIC_OPTIMIZER_DIRECTIVE_CHAIN_H
