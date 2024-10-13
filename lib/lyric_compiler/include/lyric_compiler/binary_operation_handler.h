#ifndef LYRIC_COMPILER_BINARY_OPERATION_HANDLER_H
#define LYRIC_COMPILER_BINARY_OPERATION_HANDLER_H

#include <lyric_schema/ast_schema.h>

#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    class BinaryOperationHandler : public BaseGrouping {
    public:
        BinaryOperationHandler(
            lyric_schema::LyricAstId astId,
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
        lyric_schema::LyricAstId m_astId;
        bool m_isSideEffect;
        lyric_assembler::CodeFragment *m_fragment;
    };
}

#endif // LYRIC_COMPILER_BINARY_OPERATION_HANDLER_H
