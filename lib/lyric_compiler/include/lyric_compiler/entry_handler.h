#ifndef LYRIC_COMPILER_ENTRY_HANDLER_H
#define LYRIC_COMPILER_ENTRY_HANDLER_H

#include <lyric_assembler/call_symbol.h>

#include "base_grouping.h"
#include "block_handler.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    class EntryHandler : public BaseGrouping {
    public:
        explicit EntryHandler(CompilerScanDriver *driver);

        tempo_utils::Status before(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            BeforeContext &ctx) override;

        tempo_utils::Status after(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            AfterContext &ctx) override;
    };
}

#endif // LYRIC_COMPILER_ENTRY_HANDLER_H