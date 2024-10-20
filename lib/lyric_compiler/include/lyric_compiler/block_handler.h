#ifndef LYRIC_COMPILER_BLOCK_HANDLER_H
#define LYRIC_COMPILER_BLOCK_HANDLER_H

#include <lyric_assembler/call_symbol.h>

#include "base_grouping.h"
#include "compiler_scan_driver.h"
#include "form_handler.h"

namespace lyric_compiler {

    class BlockHandler : public BaseGrouping {
    public:
        BlockHandler(
            std::unique_ptr<lyric_assembler::BlockHandle> &&block,
            bool requiresResult,
            bool isSideEffect,
            lyric_assembler::CodeFragment *fragment,
            CompilerScanDriver *driver);

        tempo_utils::Status before(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            BeforeContext &ctx) override;

        tempo_utils::Status after(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            AfterContext &ctx) override;

    private:
        std::unique_ptr<lyric_assembler::BlockHandle> m_block;
        bool m_requiresResult;
        bool m_isSideEffect;
        lyric_assembler::CodeFragment *m_fragment;
    };
}

#endif // LYRIC_COMPILER_BLOCK_HANDLER_H
