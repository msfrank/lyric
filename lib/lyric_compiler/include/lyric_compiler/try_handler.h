#ifndef LYRIC_COMPILER_TRY_HANDLER_H
#define LYRIC_COMPILER_TRY_HANDLER_H

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    struct Exception {
        lyric_common::TypeDef exceptionType;
        lyric_assembler::CodeFragment *fragment;
        std::unique_ptr<lyric_assembler::BlockHandle> block;
        lyric_assembler::JumpTarget exceptionExit;
    };

    struct TryCatch {
        lyric_assembler::CheckHandle *checkHandle = nullptr;
        lyric_assembler::DataReference caughtRef;
        std::vector<std::unique_ptr<Exception>> exceptions;
        lyric_assembler::JumpLabel alternativeLabel;
        lyric_assembler::JumpTarget alternativeJump;
        lyric_assembler::JumpLabel finallyLabel;
        lyric_assembler::JumpTarget finallyJump;
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
        lyric_assembler::CodeFragment *m_fragment;
        TryCatch m_tryCatch;
    };

    class CatchWhen : public BaseGrouping {
    public:
        CatchWhen(
            TryCatch *tryCatch,
            Exception *exception,
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
        TryCatch *m_tryCatch;
        Exception *m_exception;
    };

    class CatchPredicate : public BaseChoice {
    public:
        CatchPredicate(
            TryCatch *tryCatch,
            Exception *exception,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        TryCatch *m_tryCatch;
        Exception *m_exception;
    };

    class CatchUnpackPredicate : public BaseChoice {
    public:
        CatchUnpackPredicate(
            TryCatch *tryCatch,
            Exception *exception,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        TryCatch *m_tryCatch;
        Exception *m_exception;
    };

    class CatchBody : public BaseChoice {
    public:
        CatchBody(
            Exception *exception,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        Exception *m_exception;
    };

    class CatchAlternative : public BaseChoice {
    public:
        CatchAlternative(
            TryCatch *tryCatch,
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        TryCatch *m_tryCatch;
        lyric_assembler::CodeFragment *m_fragment;
    };
}

#endif // LYRIC_COMPILER_TRY_HANDLER_H