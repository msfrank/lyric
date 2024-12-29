#ifndef LYRIC_COMPILER_DEFALIAS_HANDLER_H
#define LYRIC_COMPILER_DEFALIAS_HANDLER_H

#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/namespace_symbol.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    class DefAliasHandler : public BaseGrouping {
    public:
        DefAliasHandler(bool isSideEffect, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);
        DefAliasHandler(
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
        lyric_assembler::BindingSymbol *m_bindingSymbol;
    };
}

#endif // LYRIC_COMPILER_DEFALIAS_HANDLER_H
