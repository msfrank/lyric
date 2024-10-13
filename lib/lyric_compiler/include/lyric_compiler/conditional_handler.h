#ifndef LYRIC_COMPILER_CONDITIONAL_HANDLER_H
#define LYRIC_COMPILER_CONDITIONAL_HANDLER_H

#include <lyric_schema/ast_schema.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"
#include "form_handler.h"

namespace lyric_compiler {

    struct WhenConsequent {
        lyric_assembler::JumpLabel jumpLabel;
        lyric_common::TypeDef predicateType;
        lyric_common::TypeDef expressionType;
        lyric_assembler::JumpTarget jumpTarget;
        lyric_assembler::CondWhenPatch patch;
    };

    struct WhenAlternative {
        lyric_assembler::JumpLabel enterLabel;
        lyric_common::TypeDef expressionType;
    };

    struct Conditional {
        std::vector<std::unique_ptr<WhenConsequent>> consequents;
        std::unique_ptr<WhenAlternative> alternative;
    };

    class IfHandler : public BaseGrouping {
    public:
        IfHandler(
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
        Conditional m_conditional;
    };

    class CondHandler : public BaseGrouping {
    public:
        CondHandler(
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
        Conditional m_conditional;
    };

    class WhenHandler : public BaseGrouping {
    public:
        WhenHandler(
            WhenConsequent *consequent,
            lyric_assembler::CodeFragment *fragment,
            bool isExpression,
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
        WhenConsequent *m_consequent;
        lyric_assembler::CodeFragment *m_fragment;
        bool m_isExpression;
    };

    class WhenBody : public BaseChoice {
    public:
        WhenBody(
            WhenConsequent *consequent,
            lyric_assembler::CodeFragment *fragment,
            bool isExpression,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        WhenConsequent *m_consequent;
        lyric_assembler::CodeFragment *m_fragment;
        bool m_isExpression;
    };

    class WhenDefault : public BaseChoice {
    public:
        WhenDefault(
            WhenAlternative *alternative,
            lyric_assembler::CodeFragment *fragment,
            bool isExpression,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        WhenAlternative *m_alternative;
        lyric_assembler::CodeFragment *m_fragment;
        bool m_isExpression;
    };
}

#endif // LYRIC_COMPILER_CONDITIONAL_HANDLER_H
