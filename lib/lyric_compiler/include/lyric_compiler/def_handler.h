#ifndef LYRIC_COMPILER_DEF_HANDLER_H
#define LYRIC_COMPILER_DEF_HANDLER_H

#include <lyric_assembler/call_symbol.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    struct Function {
        lyric_assembler::CallSymbol *callSymbol = nullptr;
        lyric_typing::TemplateSpec templateSpec;
        lyric_assembler::ParameterPack parameterPack;
        lyric_typing::TypeSpec returnSpec;
        lyric_assembler::ProcHandle *procHandle = nullptr;
    };

    class DefHandler : public BaseGrouping {
    public:
        DefHandler(bool isSideEffect, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);
        DefHandler(
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
        Function m_function;
        bool m_isSideEffect;
        lyric_assembler::NamespaceSymbol *m_currentNamespace;
    };

    class DefProc : public BaseChoice {
    public:
        DefProc(Function *function, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        Function *m_function;
    };
}

#endif // LYRIC_COMPILER_DEF_HANDLER_H
