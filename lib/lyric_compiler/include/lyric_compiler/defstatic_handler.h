#ifndef LYRIC_COMPILER_DEFSTATIC_HANDLER_H
#define LYRIC_COMPILER_DEFSTATIC_HANDLER_H

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/impl_handle.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    class DefStaticHandler : public BaseGrouping {
    public:
        DefStaticHandler(bool isSideEffect, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);
        DefStaticHandler(
            bool isSideEffect,
            lyric_assembler::NamespaceSymbol *currentNamespace,
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
        lyric_assembler::NamespaceSymbol *m_currentNamespace;
        lyric_assembler::StaticSymbol *m_staticSymbol;
        lyric_assembler::ProcHandle *m_procHandle;
    };
}

#endif // LYRIC_COMPILER_DEFSTATIC_HANDLER_H
