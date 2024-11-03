#ifndef LYRIC_COMPILER_ACTION_HANDLER_H
#define LYRIC_COMPILER_ACTION_HANDLER_H

#include <lyric_assembler/action_symbol.h>
#include <lyric_typing/callsite_reifier.h>

#include "base_grouping.h"
#include "base_invokable_handler.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    struct Action {
        lyric_assembler::ActionSymbol *actionSymbol = nullptr;
    };

    class ActionHandler : public BaseGrouping {
    public:
        ActionHandler(
            Action method,
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
        Action m_action;
    };
}

#endif // LYRIC_COMPILER_ACTION_HANDLER_H
