#ifndef LYRIC_REWRITER_VISITOR_REGISTRY_H
#define LYRIC_REWRITER_VISITOR_REGISTRY_H

#include <absl/container/flat_hash_map.h>
#include <tempo_utils/url.h>

#include "rewrite_processor.h"

namespace lyric_rewriter {

    class VisitorRegistry {
    public:
        explicit VisitorRegistry(bool excludePredefinedNamespaces = false);

        using MakeVisitorFunc = std::function<std::shared_ptr<AbstractNodeVisitor>(
            const lyric_parser::ArchetypeNode *,
            AbstractProcessorState *)>;

        tempo_utils::Status registerVisitorNamespace(const tempo_utils::Url &nsUrl, MakeVisitorFunc func);
        tempo_utils::Status replaceVisitorNamespace(const tempo_utils::Url &nsUrl, MakeVisitorFunc func);
        tempo_utils::Status deregisterVisitorNamespace(const tempo_utils::Url &nsUrl);
        tempo_utils::Status setMakeUnknownVisitorFunc(MakeVisitorFunc func);

        void sealRegistry();

        tempo_utils::Result<std::shared_ptr<AbstractNodeVisitor>> makeVisitor(
            const lyric_parser::ArchetypeNode *node,
            AbstractProcessorState *state) const;

    private:
        absl::flat_hash_map<tempo_utils::Url, MakeVisitorFunc> m_makeVisitorFuncs;
        MakeVisitorFunc m_makeUnknownVisitorFunc;
        bool m_isSealed;
    };
}

#endif // LYRIC_REWRITER_VISITOR_REGISTRY_H
