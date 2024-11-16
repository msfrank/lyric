#ifndef LYRIC_COMPILER_ASSEMBLER_HANDLER_H
#define LYRIC_COMPILER_ASSEMBLER_HANDLER_H

#include <lyric_schema/ast_schema.h>

#include "abstract_behavior.h"
#include "base_choice.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    class AssemblerChoice : public BaseChoice {
    public:
        AssemblerChoice(
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        lyric_assembler::CodeFragment *m_fragment;
    };
}

#endif // LYRIC_COMPILER_ASSEMBLER_HANDLER_H
