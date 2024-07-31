#ifndef LYRIC_REWRITER_SCHEMA_CLASS_NODE_VISITOR_H
#define LYRIC_REWRITER_SCHEMA_CLASS_NODE_VISITOR_H

#include "rewrite_processor.h"
#include "rewriter_result.h"

namespace lyric_rewriter {

    template<class NsType, class IdType>
    class SchemaClassNodeVisitor : public AbstractNodeVisitor {
    public:
        explicit SchemaClassNodeVisitor(
            const tempo_utils::SchemaVocabulary<NsType,IdType> &vocabulary,
            std::shared_ptr<AbstractNodeVisitor> unknownVisitor = {})
            : m_schemaNs(vocabulary.getNs()),
              m_unknownVisitor(unknownVisitor)
        {
            TU_ASSERT (m_schemaNs != nullptr);
        };

        void
        putVisitor(
            const tempo_utils::SchemaClass<NsType, IdType> &schemaClass,
            std::shared_ptr<AbstractNodeVisitor> visitor)
        {
            m_idVisitors[schemaClass.getIdValue()] = visitor;
        };

        tempo_utils::Status
        enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx) override
        {
            if (node->isNamespace(*m_schemaNs)) {
                auto idValue = node->getTypeOffset();
                auto entry = m_idVisitors.find(idValue);
                if (entry != m_idVisitors.cend())
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
            if (node->isNamespace(*m_schemaNs)) {
                auto idValue = node->getTypeOffset();
                auto entry = m_idVisitors.find(idValue);
                if (entry != m_idVisitors.cend())
                    return entry->second->exit(node, ctx);
            } else if (m_unknownVisitor != nullptr) {
                return m_unknownVisitor->exit(node, ctx);
            }
            return RewriterStatus::forCondition(
                RewriterCondition::kRewriterInvariant, "unknown node");
        };

    private:
        const NsType *m_schemaNs;
        std::shared_ptr<AbstractNodeVisitor> m_unknownVisitor;
        absl::flat_hash_map<tu_uint32,std::shared_ptr<AbstractNodeVisitor>> m_idVisitors;
    };
}

#endif // LYRIC_REWRITER_SCHEMA_CLASS_NODE_VISITOR_H
