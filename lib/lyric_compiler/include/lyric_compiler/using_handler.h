#ifndef LYRIC_COMPILER_USING_HANDLER_H
#define LYRIC_COMPILER_USING_HANDLER_H

#include <lyric_assembler/call_symbol.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    class UsingHandler : public BaseGrouping {
    public:
        UsingHandler(
            bool isSideEffect,
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
        lyric_common::ModuleLocation m_usingLocation;
    };

    class UsingImpl : public BaseChoice {
    public:
        UsingImpl(
            const lyric_common::ModuleLocation &usingLocation,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        lyric_common::ModuleLocation m_usingLocation;
    };
}

#endif // LYRIC_COMPILER_USING_HANDLER_H
