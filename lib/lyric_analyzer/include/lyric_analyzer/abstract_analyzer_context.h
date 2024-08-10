#ifndef LYRIC_ANALYZER_ABSTRACT_ANALYZER_CONTEXT_H
#define LYRIC_ANALYZER_ABSTRACT_ANALYZER_CONTEXT_H

#include <lyric_assembler/block_handle.h>
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_rewriter/rewrite_processor.h>

namespace lyric_analyzer {

    class AbstractAnalyzerContext {
    public:
        virtual ~AbstractAnalyzerContext() = default;

        virtual lyric_assembler::BlockHandle *getBlock() const = 0;

        virtual tempo_utils::Status enter(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_rewriter::VisitorContext &ctx) = 0;

        virtual tempo_utils::Status exit(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            const lyric_rewriter::VisitorContext &ctx) = 0;
    };
}

#endif // LYRIC_ANALYZER_ABSTRACT_ANALYZER_CONTEXT_H
