#ifndef LYRIC_COMPILER_TARGET_HANDLER_H
#define LYRIC_COMPILER_TARGET_HANDLER_H

#include "base_choice.h"
#include "base_grouping.h"
#include "form_handler.h"

namespace lyric_compiler {

    struct Target {
        lyric_assembler::DataReference targetRef;
        lyric_common::TypeDef receiverType;
        bool thisReceiver = false;
        lyric_assembler::BlockHandle *bindingBlock = nullptr;
    };

    class TargetHandler : public BaseGrouping {
    public:
        TargetHandler(
            Target *target,
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
        Target *m_target;
        lyric_assembler::CodeFragment *m_fragment;
    };

    class InitialTarget : public AbstractBehavior {
    public:
        InitialTarget(Target *target, lyric_assembler::CodeFragment *fragment);

        tempo_utils::Status enter(
            CompilerScanDriver *driver,
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_assembler::BlockHandle *block,
            EnterContext &ctx) override;
        tempo_utils::Status exit(
            CompilerScanDriver *driver,
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_assembler::BlockHandle *block,
            ExitContext &ctx) override;

    private:
        Target *m_target;
        lyric_assembler::CodeFragment *m_fragment;
    };

    class ResolveMember : public AbstractBehavior {
    public:
        ResolveMember(Target *target, lyric_assembler::CodeFragment *fragment);

        tempo_utils::Status enter(
            CompilerScanDriver *driver,
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_assembler::BlockHandle *block,
            EnterContext &ctx) override;
        tempo_utils::Status exit(
            CompilerScanDriver *driver,
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_assembler::BlockHandle *block,
            ExitContext &ctx) override;

    private:
        Target *m_target;
        lyric_assembler::CodeFragment *m_fragment;
    };

    class ResolveTarget : public AbstractBehavior {
    public:
        explicit ResolveTarget(Target *target);

        tempo_utils::Status enter(
            CompilerScanDriver *driver,
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_assembler::BlockHandle *block,
            EnterContext &ctx) override;
        tempo_utils::Status exit(
            CompilerScanDriver *driver,
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_assembler::BlockHandle *block,
            ExitContext &ctx) override;

    private:
        Target *m_target;
    };
}

#endif // LYRIC_COMPILER_TARGET_HANDLER_H
