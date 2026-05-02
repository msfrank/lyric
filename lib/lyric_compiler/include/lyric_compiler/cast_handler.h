#ifndef LYRIC_COMPILER_CAST_HANDLER_H
#define LYRIC_COMPILER_CAST_HANDLER_H

#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    class CastHandler : public BaseGrouping {
    public:
        CastHandler(
            bool isSideEffect,
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
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
        bool m_isSideEffect;
        lyric_assembler::CodeFragment *m_fragment;
    };
}

#endif // LYRIC_COMPILER_CAST_HANDLER_H
