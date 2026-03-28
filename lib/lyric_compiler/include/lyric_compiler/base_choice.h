#ifndef LYRIC_COMPILER_BASE_CHOICE_H
#define LYRIC_COMPILER_BASE_CHOICE_H

#include <lyric_assembler/block_handle.h>
#include <lyric_parser/archetype_node.h>

#include "visitor_context.h"

namespace lyric_compiler {

    class BaseChoice : public AbstractChoice {
    public:
        explicit BaseChoice(CompilerScanDriver *driver);
        BaseChoice(lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        lyric_assembler::BlockHandle *getBlock() override;
        CompilerScanDriver *getDriver() override;

    private:
        lyric_assembler::BlockHandle *m_block;
        CompilerScanDriver *m_driver;
    };
}

#endif // LYRIC_COMPILER_BASE_CHOICE_H
