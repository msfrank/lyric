#ifndef LYRIC_COMPILER_LAMBDA_HANDLER_H
#define LYRIC_COMPILER_LAMBDA_HANDLER_H

#include <lyric_assembler/call_symbol.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    struct Lambda {
        lyric_assembler::CallSymbol *callSymbol = nullptr;
        lyric_typing::TemplateSpec templateSpec;
        lyric_assembler::ParameterPack parameterPack;
        lyric_typing::TypeSpec returnSpec;
        lyric_assembler::ProcHandle *procHandle = nullptr;
    };

    class LambdaHandler : public BaseGrouping {
    public:
        LambdaHandler(
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
        Lambda m_lambda;
        bool m_isSideEffect;
        lyric_assembler::CodeFragment *m_fragment;
    };

    class LambdaProc : public BaseChoice {
    public:
        LambdaProc(Lambda *lambda, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        Lambda *m_lambda;
    };

    class LambdaFrom : public BaseChoice {
    public:
        LambdaFrom(
            bool isSideEffect,
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        Lambda m_lambda;
        bool m_isSideEffect;
        lyric_assembler::CodeFragment *m_fragment;
    };
}

#endif // LYRIC_COMPILER_LAMBDA_HANDLER_H
