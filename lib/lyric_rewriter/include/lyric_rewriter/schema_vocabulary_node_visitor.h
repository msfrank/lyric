#ifndef LYRIC_REWRITER_SCHEMA_VOCABULARY_NODE_VISITOR_H
#define LYRIC_REWRITER_SCHEMA_VOCABULARY_NODE_VISITOR_H

#include <tempo_utils/schema.h>

#include "rewrite_processor.h"
#include "rewriter_result.h"

namespace lyric_rewriter {

    class SchemaVocabularyNodeVisitor : public AbstractNodeVisitor {
    public:
        explicit SchemaVocabularyNodeVisitor(std::shared_ptr<AbstractNodeVisitor> unknownVisitor = {})
            : m_unknownVisitor(unknownVisitor)
        {
        };

        void
        putVisitor(
            const tempo_utils::SchemaNs &schemaNs,
            std::shared_ptr<AbstractNodeVisitor> visitor)
        {
            std::string nsString{schemaNs.getNs()};
            m_nsVisitors[nsString] = visitor;
        };

        template<class NsType, class IdType>
        void
        putVisitor(
            const tempo_utils::SchemaVocabulary<NsType, IdType> &schemaVocabulary,
            std::shared_ptr<AbstractNodeVisitor> visitor)
        {
            auto *schemaNs = schemaVocabulary.getNs();
            putVisitor(*schemaNs);
        };

        tempo_utils::Status
        enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx) override
        {
            auto entry = m_nsVisitors.find(node->namespaceView());
            if (entry != m_nsVisitors.cend()) {
                return entry->second->enter(node, ctx);
            } else if (m_unknownVisitor != nullptr) {
                return m_unknownVisitor->enter(node, ctx);
            }
            return RewriterStatus::forCondition(
                RewriterCondition::kRewriterInvariant, "unknown node");
        };

        tempo_utils::Status
        exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx) override
        {
            auto entry = m_nsVisitors.find(node->namespaceView());
            if (entry != m_nsVisitors.cend()) {
                return entry->second->exit(node, ctx);
            } else if (m_unknownVisitor != nullptr) {
                return m_unknownVisitor->exit(node, ctx);
            }
            return RewriterStatus::forCondition(
                RewriterCondition::kRewriterInvariant, "unknown node");
        };

    private:
        std::shared_ptr<AbstractNodeVisitor> m_unknownVisitor;
        absl::flat_hash_map<std::string,std::shared_ptr<AbstractNodeVisitor>> m_nsVisitors;
    };
}

#endif // LYRIC_REWRITER_SCHEMA_VOCABULARY_NODE_VISITOR_H
