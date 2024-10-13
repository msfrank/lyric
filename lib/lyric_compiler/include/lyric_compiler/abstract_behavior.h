#ifndef LYRIC_COMPILER_ABSTRACT_BEHAVIOR_H
#define LYRIC_COMPILER_ABSTRACT_BEHAVIOR_H

#include <lyric_assembler/block_handle.h>
#include <lyric_parser/archetype_node.h>

#include "compiler_scan_driver.h"
#include "visitor_context.h"

namespace lyric_compiler {

    class AbstractBehavior {
    public:
        virtual ~AbstractBehavior() = default;

        virtual tempo_utils::Status enter(
            CompilerScanDriver *driver,
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_assembler::BlockHandle *block,
            EnterContext &ctx) = 0;

        virtual tempo_utils::Status exit(
            CompilerScanDriver *driver,
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_assembler::BlockHandle *block,
            ExitContext &ctx) = 0;
    };
}

#endif // LYRIC_COMPILER_ABSTRACT_BEHAVIOR_H
