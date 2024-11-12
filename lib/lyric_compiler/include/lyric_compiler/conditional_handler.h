#ifndef LYRIC_COMPILER_CONDITIONAL_HANDLER_H
#define LYRIC_COMPILER_CONDITIONAL_HANDLER_H

#include <lyric_schema/ast_schema.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"
#include "form_handler.h"

namespace lyric_compiler {

    struct ConditionalConsequent {
        lyric_assembler::JumpLabel jumpLabel;
        lyric_common::TypeDef predicateType;
        lyric_common::TypeDef consequentType;
        lyric_assembler::JumpTarget jumpTarget;
        lyric_assembler::CondWhenPatch patch;
    };

    struct ConditionalAlternative {
        lyric_assembler::JumpLabel enterLabel;
        lyric_common::TypeDef alternativeType;
    };

    struct Conditional {
        std::vector<std::unique_ptr<ConditionalConsequent>> consequents;
        std::unique_ptr<ConditionalAlternative> alternative;
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

    class ConditionalWhen : public BaseGrouping {
    public:
        ConditionalWhen(
            ConditionalConsequent *consequent,
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
        ConditionalConsequent *m_consequent;
        lyric_assembler::CodeFragment *m_fragment;
        bool m_isExpression;
    };

    class ConditionalBody : public BaseChoice {
    public:
        ConditionalBody(
            ConditionalConsequent *consequent,
            lyric_assembler::CodeFragment *fragment,
            bool isExpression,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        ConditionalConsequent *m_consequent;
        lyric_assembler::CodeFragment *m_fragment;
        bool m_isExpression;
    };

    class ConditionalDefault : public BaseChoice {
    public:
        ConditionalDefault(
            ConditionalAlternative *alternative,
            lyric_assembler::CodeFragment *fragment,
            bool isExpression,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        ConditionalAlternative *m_alternative;
        lyric_assembler::CodeFragment *m_fragment;
        bool m_isExpression;
    };
}

#endif // LYRIC_COMPILER_CONDITIONAL_HANDLER_H
