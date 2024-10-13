#ifndef LYRIC_COMPILER_ASSIGNMENT_HANDLER_H
#define LYRIC_COMPILER_ASSIGNMENT_HANDLER_H

#include <lyric_schema/ast_schema.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"
#include "form_handler.h"

namespace lyric_compiler {

    struct Assignment {
        lyric_assembler::BlockHandle *bindingBlock = nullptr;
        lyric_schema::LyricAstId astId;
        bool thisReceiver = false;
        lyric_common::TypeDef receiverType;
        std::unique_ptr<lyric_assembler::CodeFragment> resolveTarget;
        lyric_assembler::DataReference targetRef;
        std::unique_ptr<lyric_assembler::CodeFragment> evaluateExpression;
        lyric_common::TypeDef expressionType;
    };

    class AssignmentHandler : public BaseGrouping {
    public:
        AssignmentHandler(
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
        Assignment m_assignment;
    };

    class AssignmentTarget : public BaseChoice {
    public:
        AssignmentTarget(Assignment *assignment, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        Assignment *m_assignment;
    };

    class AssignmentTargetHandler : public BaseGrouping {
    public:
        AssignmentTargetHandler(
            lyric_assembler::BlockHandle *block,
            Assignment *assignment,
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
        Assignment *m_assignment;
    };

    class InitialTarget : public AbstractBehavior {
    public:
        explicit InitialTarget(Assignment *assignment);

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
        Assignment *m_assignment;
    };

    class ResolveMember : public AbstractBehavior {
    public:
        explicit ResolveMember(Assignment *assignment);

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
        Assignment *m_assignment;
    };

    class ResolveTarget : public AbstractBehavior {
    public:
        explicit ResolveTarget(Assignment *assignment);

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
        Assignment *m_assignment;
    };
}

#endif // LYRIC_COMPILER_ASSIGNMENT_HANDLER_H
