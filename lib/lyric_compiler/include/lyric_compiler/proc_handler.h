#ifndef LYRIC_COMPILER_PROC_HANDLER_H
#define LYRIC_COMPILER_PROC_HANDLER_H

#include <lyric_assembler/call_symbol.h>

#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    class ProcHandler : public BaseGrouping {
    public:
        ProcHandler(
            lyric_assembler::ProcHandle *procHandle,
            bool requiresResult,
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
        lyric_assembler::ProcHandle *m_procHandle;
        bool m_requiresResult;
    };
}

#endif // LYRIC_COMPILER_PROC_HANDLER_H