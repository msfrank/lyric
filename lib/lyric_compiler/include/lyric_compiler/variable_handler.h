#ifndef LYRIC_COMPILER_VARIABLE_HANDLER_H
#define LYRIC_COMPILER_VARIABLE_HANDLER_H

#include <lyric_schema/ast_schema.h>

#include "base_grouping.h"
#include "compiler_scan_driver.h"
#include "form_handler.h"

namespace lyric_compiler {

    class VariableHandler : public BaseGrouping {
    public:
        VariableHandler(
            bool isVariable,
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
        bool m_isVariable;
        bool m_isSideEffect;
        lyric_assembler::CodeFragment *m_fragment;
        lyric_common::TypeDef m_typeHint;
    };
}

#endif // LYRIC_COMPILER_VARIABLE_HANDLER_H
