#ifndef LYRIC_COMPILER_MATCH_HANDLER_H
#define LYRIC_COMPILER_MATCH_HANDLER_H

#include <lyric_schema/ast_schema.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"
#include "form_handler.h"

namespace lyric_compiler {

    struct MatchConsequent {
        lyric_assembler::JumpLabel jumpLabel;
        lyric_common::TypeDef predicateType;
        lyric_common::TypeDef consequentType;
        lyric_assembler::JumpTarget jumpTarget;
        std::unique_ptr<lyric_assembler::BlockHandle> block;
        lyric_assembler::MatchWhenPatch patch;
    };

    struct MatchAlternative {
        lyric_assembler::JumpLabel enterLabel;
        lyric_common::TypeDef alternativeType;
    };

    struct Match {
        lyric_assembler::DataReference targetRef;
        std::vector<std::unique_ptr<MatchConsequent>> consequents;
        std::unique_ptr<MatchAlternative> alternative;
    };

    class MatchHandler : public BaseGrouping {
    public:
        MatchHandler(
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
        Match m_match;
    };

    class MatchInitial : public BaseChoice {
    public:
        MatchInitial(
            Match *match,
            MatchConsequent *consequent,
            lyric_assembler::CodeFragment *fragment,
            bool isExpression,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        Match *m_match;
        MatchConsequent *m_consequent;
        lyric_assembler::CodeFragment *m_fragment;
        bool m_isExpression;
    };

    class MatchWhen : public BaseGrouping {
    public:
        MatchWhen(
            Match *match,
            MatchConsequent *consequent,
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
        Match *m_match;
        MatchConsequent *m_consequent;
        lyric_assembler::CodeFragment *m_fragment;
        bool m_isExpression;
    };

    class MatchPredicate : public BaseChoice {
    public:
        MatchPredicate(
            Match *match,
            MatchConsequent *consequent,
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        Match *m_match;
        MatchConsequent *m_consequent;
        lyric_assembler::CodeFragment *m_fragment;
    };

    class MatchUnpackPredicate : public BaseChoice {
    public:
        MatchUnpackPredicate(
            Match *match,
            MatchConsequent *consequent,
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        Match *m_match;
        MatchConsequent *m_consequent;
        lyric_assembler::CodeFragment *m_fragment;
    };

    class MatchSymbolRefPredicate : public BaseChoice {
    public:
        MatchSymbolRefPredicate(
            Match *match,
            MatchConsequent *consequent,
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        Match *m_match;
        MatchConsequent *m_consequent;
        lyric_assembler::CodeFragment *m_fragment;
    };

    class MatchBody : public BaseChoice {
    public:
        MatchBody(
            MatchConsequent *consequent,
            lyric_assembler::CodeFragment *fragment,
            bool isExpression,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        MatchConsequent *m_consequent;
        lyric_assembler::CodeFragment *m_fragment;
        bool m_isExpression;
    };

    class MatchDefault : public BaseChoice {
    public:
        MatchDefault(
            MatchAlternative *alternative,
            lyric_assembler::CodeFragment *fragment,
            bool isExpression,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        MatchAlternative *m_alternative;
        lyric_assembler::CodeFragment *m_fragment;
        bool m_isExpression;
    };
}

#endif // LYRIC_COMPILER_MATCH_HANDLER_H
