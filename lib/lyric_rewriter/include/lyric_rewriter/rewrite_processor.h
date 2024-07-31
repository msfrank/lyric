#ifndef LYRIC_REWRITER_REWRITE_PROCESSOR_H
#define LYRIC_REWRITER_REWRITE_PROCESSOR_H

#include <vector>

#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <tempo_utils/schema.h>

namespace lyric_rewriter {

    // forward declarations
    class AbstractNodeVisitor;

    struct Visitation {
        int index;
        lyric_parser::ArchetypeNode *node;
        std::shared_ptr<AbstractNodeVisitor> visitor;
        bool seen;
    };

    class VisitorContext {
    public:
        VisitorContext(
            lyric_parser::ArchetypeState *state,
            std::shared_ptr<AbstractNodeVisitor> curr,
            lyric_parser::ArchetypeNode *parent,
            int index);

        lyric_parser::ArchetypeState *archetypeState() const;
        int childIndex() const;
        lyric_parser::ArchetypeNode *parentNode() const;
        bool skipChildren() const;

        void setSkipChildren(bool skip);
        void push(
            int childIndex,
            lyric_parser::ArchetypeNode *childNode,
            std::shared_ptr<AbstractNodeVisitor> visitor = {});

    private:
        lyric_parser::ArchetypeState *m_state;
        std::shared_ptr<AbstractNodeVisitor> m_curr;
        lyric_parser::ArchetypeNode *m_parent;
        int m_index;
        std::vector<Visitation> m_added;
        bool m_skip;

        void extend(std::vector<Visitation> &stack);
        friend class RewriteProcessor;
    };

    class AbstractNodeVisitor {
    public:
        virtual ~AbstractNodeVisitor() = default;
        virtual tempo_utils::Status enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx) = 0;
        virtual tempo_utils::Status exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx) = 0;
    };

    class RewriteProcessor {
    public:
        RewriteProcessor();

        tempo_utils::Status process(
            lyric_parser::ArchetypeState *state,
            std::shared_ptr<AbstractNodeVisitor> visitor);

    private:
        std::vector<Visitation> m_stack;

        bool isEmpty();
        Visitation& peek();
        lyric_parser::ArchetypeNode *parentNode();
        void pop();
    };
}

#endif // LYRIC_REWRITER_REWRITE_PROCESSOR_H
