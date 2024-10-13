#ifndef LYRIC_COMPILER_BASE_CHOICE_H
#define LYRIC_COMPILER_BASE_CHOICE_H

#include <lyric_assembler/block_handle.h>
#include <lyric_schema/ast_schema.h>

#include "abstract_behavior.h"
#include "base_grouping.h"
#include "visitor_context.h"

namespace lyric_compiler {

    class BaseChoice {
    public:
        explicit BaseChoice(CompilerScanDriver *driver);
        BaseChoice(lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);
        virtual ~BaseChoice() = default;

        virtual tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) = 0;

        lyric_assembler::BlockHandle *getBlock();
        CompilerScanDriver *getDriver();


    private:
        lyric_assembler::BlockHandle *m_block;
        CompilerScanDriver *m_driver;
    };
}

#endif // LYRIC_COMPILER_BASE_CHOICE_H
