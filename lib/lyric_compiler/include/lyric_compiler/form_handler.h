#ifndef LYRIC_COMPILER_FORM_HANDLER_H
#define LYRIC_COMPILER_FORM_HANDLER_H

#include <lyric_schema/ast_schema.h>

#include "abstract_behavior.h"
#include "base_choice.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    class TerminalFormBehavior : public AbstractBehavior {
    public:
        TerminalFormBehavior(
            bool isSideEffect,
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

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
        bool m_isSideEffect;
        lyric_assembler::CodeFragment *m_fragment;
        CompilerScanDriver *m_driver;
        lyric_assembler::BlockHandle *m_block;
    };

    enum class FormType {
        Any,
        SideEffect,
        Expression,
        Statement,
    };

    class FormChoice : public BaseChoice {
    public:
        FormChoice(
            FormType type,
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        FormType m_type;
        lyric_assembler::CodeFragment *m_fragment;
    };
}

#endif // LYRIC_COMPILER_FORM_HANDLER_H
