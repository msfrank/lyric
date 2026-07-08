#ifndef LYRIC_COMPILER_ROOT_HANDLER_H
#define LYRIC_COMPILER_ROOT_HANDLER_H

#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    class RootHandler : public BaseGrouping {
    public:
        explicit RootHandler(CompilerScanDriver *driver);

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

#endif // LYRIC_COMPILER_ROOT_HANDLER_H
