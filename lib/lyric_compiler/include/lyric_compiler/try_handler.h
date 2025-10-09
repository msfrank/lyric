#ifndef LYRIC_COMPILER_TRY_HANDLER_H
#define LYRIC_COMPILER_TRY_HANDLER_H

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    struct TryCatchFinally {
        lyric_assembler::CheckHandle *checkHandle = nullptr;
        lyric_assembler::CleanupHandle *cleanupHandle = nullptr;
        lyric_assembler::CodeFragment *fragment = nullptr;
        lyric_assembler::JumpTarget checkExit;
        lyric_assembler::JumpLabel afterCatch;
    };

    class TryHandler : public BaseGrouping {
    public:
        TryHandler(
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
        TryCatchFinally m_tryCatchFinally;
    };

    class TryCatch : public BaseGrouping {
    public:
        TryCatch(
            TryCatchFinally *tryCatchFinally,
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
        TryCatchFinally *m_tryCatchFinally;
    };

    class TryFinally : public BaseGrouping {
    public:
        TryFinally(
            TryCatchFinally *tryCatchFinally,
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
        TryCatchFinally *m_tryCatchFinally;
    };

    struct Exception {
        lyric_assembler::CatchHandle *catchHandle = nullptr;
        std::unique_ptr<lyric_assembler::BlockHandle> block;
    };

    class CatchWhen : public BaseGrouping {
    public:
        CatchWhen(
            TryCatchFinally *tryCatchFinally,
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
        TryCatchFinally *m_tryCatchFinally;
        Exception m_exception;
    };

    class CatchPredicate : public BaseChoice {
    public:
        CatchPredicate(
            TryCatchFinally *tryCatchFinally,
            Exception *exception,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        TryCatchFinally *m_tryCatchFinally;
        Exception *m_exception;
    };

    class CatchUnpackPredicate : public BaseChoice {
    public:
        CatchUnpackPredicate(
            TryCatchFinally *tryCatchFinally,
            Exception *exception,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        TryCatchFinally *m_tryCatchFinally;
        Exception *m_exception;
    };

    class CatchBody : public BaseChoice {
    public:
        CatchBody(
            Exception *exception,
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        Exception *m_exception;
        lyric_assembler::CodeFragment *m_fragment;
    };

    class CatchElse : public BaseChoice {
    public:
        CatchElse(
            TryCatchFinally *tryCatchFinally,
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        TryCatchFinally *m_tryCatchFinally;
        lyric_assembler::CodeFragment *m_fragment;
    };
}

#endif // LYRIC_COMPILER_TRY_HANDLER_H