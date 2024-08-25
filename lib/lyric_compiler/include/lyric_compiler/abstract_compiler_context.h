#ifndef LYRIC_COMPILER_ABSTRACT_COMPILER_CONTEXT_H
#define LYRIC_COMPILER_ABSTRACT_COMPILER_CONTEXT_H

#include <lyric_assembler/block_handle.h>
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_rewriter/rewrite_processor.h>

namespace lyric_compiler {

    class AbstractCompilerContext {
    public:
        virtual ~AbstractCompilerContext() = default;

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

#endif // LYRIC_COMPILER_ABSTRACT_COMPILER_CONTEXT_H
