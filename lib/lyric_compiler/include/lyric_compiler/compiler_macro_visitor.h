#ifndef LYRIC_COMPILER_COMPILER_MACRO_VISITOR_H
#define LYRIC_COMPILER_COMPILER_MACRO_VISITOR_H

#include <lyric_rewriter/rewrite_processor.h>

namespace lyric_compiler {

    class CompilerMacroVisitor : public lyric_rewriter::AbstractNodeVisitor {
    public:
        explicit CompilerMacroVisitor(lyric_rewriter::AbstractProcessorState *state);

        tempo_utils::Status enter(
            lyric_parser::ArchetypeNode *node,
            lyric_rewriter::VisitorContext &ctx) override;
        tempo_utils::Status exit(
            lyric_parser::ArchetypeNode *node,
            const lyric_rewriter::VisitorContext &ctx) override;

    private:
        lyric_rewriter::AbstractProcessorState *m_state;
    };

    std::shared_ptr<lyric_rewriter::AbstractNodeVisitor> make_compiler_visitor(
        const lyric_parser::ArchetypeNode *node,
        lyric_rewriter::AbstractProcessorState *state);
}

#endif // LYRIC_COMPILER_COMPILER_MACRO_VISITOR_H
