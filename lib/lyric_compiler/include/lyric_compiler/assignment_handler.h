#ifndef LYRIC_COMPILER_ASSIGNMENT_HANDLER_H
#define LYRIC_COMPILER_ASSIGNMENT_HANDLER_H

#include <lyric_schema/ast_schema.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"
#include "target_handler.h"

namespace lyric_compiler {

    struct Assignment {
        lyric_schema::LyricAstId astId;
        std::unique_ptr<lyric_assembler::CodeFragment> resolveTarget;
        std::unique_ptr<lyric_assembler::CodeFragment> evaluateExpression;
        lyric_common::TypeDef expressionType;
        Target target;
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
}

#endif // LYRIC_COMPILER_ASSIGNMENT_HANDLER_H
