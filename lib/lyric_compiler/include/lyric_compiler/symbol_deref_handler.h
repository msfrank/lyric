#ifndef LYRIC_COMPILER_SYMBOL_DEREF_HANDLER_H
#define LYRIC_COMPILER_SYMBOL_DEREF_HANDLER_H

#include <lyric_assembler/call_symbol.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "base_invokable_handler.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    struct SymbolDeref {
        lyric_assembler::CodeFragment *fragment = nullptr;
        lyric_assembler::BlockHandle *bindingBlock = nullptr;
    };

    class SymbolDerefHandler : public BaseGrouping {
    public:
        SymbolDerefHandler(
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
        SymbolDeref m_deref;
    };

    class SymbolDerefInitial : public BaseChoice {
    public:
        SymbolDerefInitial(SymbolDeref *deref, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        SymbolDeref *m_deref;
    };

    class SymbolDerefNext : public BaseChoice {
    public:
        SymbolDerefNext(SymbolDeref *deref, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        SymbolDeref *m_deref;
    };
}

#endif // LYRIC_COMPILER_SYMBOL_DEREF_HANDLER_H
