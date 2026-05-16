#ifndef LYRIC_COMPILER_ITERATION_HANDLER_H
#define LYRIC_COMPILER_ITERATION_HANDLER_H

#include <lyric_schema/ast_schema.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"
#include "form_handler.h"

namespace lyric_compiler {

    struct Loop {
        lyric_assembler::LoopHandle *loopHandle = nullptr;
    };

    class WhileHandler : public BaseGrouping {
    public:
        WhileHandler(
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
        lyric_assembler::CodeFragment *m_fragment;
        Loop m_loop;
    };

    class WhileBody : public BaseChoice {
    public:
        WhileBody(
            Loop *loop,
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        Loop *m_loop;
        lyric_assembler::CodeFragment *m_fragment;
    };

    struct Iteration {
        std::string targetIdentifier;
        lyric_common::TypeDef targetType;
        lyric_common::TypeDef generatorType;
        std::unique_ptr<lyric_assembler::BlockHandle> forBlock;
        lyric_assembler::LoopHandle *loopHandle = nullptr;
    };

    class ForHandler : public BaseGrouping {
    public:
        ForHandler(
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
        lyric_assembler::CodeFragment *m_fragment;
        Iteration m_iteration;
    };

    class ForBody : public BaseChoice {
    public:
        ForBody(
            Iteration *iteration,
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        Iteration *m_iteration;
        lyric_assembler::CodeFragment *m_fragment;
    };

    class ContinueHandler : public BaseChoice {
    public:
        ContinueHandler(
            bool isSideEffect,
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        bool m_isSideEffect;
        lyric_assembler::CodeFragment *m_fragment;
    };

    class BreakHandler : public BaseChoice {
    public:
        BreakHandler(
            bool isSideEffect,
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        bool m_isSideEffect;
        lyric_assembler::CodeFragment *m_fragment;
    };
}

#endif // LYRIC_COMPILER_ITERATION_HANDLER_H
