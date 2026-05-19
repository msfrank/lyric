#ifndef LYRIC_COMPILER_STUB_HANDLER_H
#define LYRIC_COMPILER_STUB_HANDLER_H

#include <lyric_assembler/action_symbol.h>

#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    struct Stub {
        lyric_assembler::ActionSymbol *actionSymbol = nullptr;
    };

    class StubHandler : public BaseGrouping {
    public:
        StubHandler(
            Stub stub,
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
        Stub m_stub;
    };
}

#endif // LYRIC_COMPILER_STUB_HANDLER_H
