#ifndef LYRIC_COMPILER_DEREF_NODE_HANDLER_H
#define LYRIC_COMPILER_DEREF_NODE_HANDLER_H

#include <lyric_assembler/call_symbol.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    struct Deref {
        lyric_assembler::CodeFragment *fragment = nullptr;
        lyric_assembler::BlockHandle *bindingBlock = nullptr;
        lyric_assembler::BlockHandle *invokeBlock = nullptr;
        bool thisReceiver = false;
    };

    class DerefNodeHandler : public BaseGrouping {
    public:
        DerefNodeHandler(
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
        Deref m_deref;
    };

    class DerefInitial : public BaseChoice {
    public:
        DerefInitial(Deref *deref, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        Deref *m_deref;
    };

    class DerefNext : public BaseChoice {
    public:
        DerefNext(Deref *deref, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        Deref *m_deref;
    };
}

#endif // LYRIC_COMPILER_DEREF_NODE_HANDLER_H
