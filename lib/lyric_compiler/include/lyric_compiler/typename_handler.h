#ifndef LYRIC_COMPILER_TYPENAME_HANDLER_H
#define LYRIC_COMPILER_TYPENAME_HANDLER_H

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    class TypenameHandler : public BaseChoice {
    public:
        TypenameHandler(bool isSideEffect, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);
        TypenameHandler(
            bool isSideEffect,
            lyric_assembler::NamespaceSymbol *currentNamespace,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        bool m_isSideEffect;
        lyric_assembler::NamespaceSymbol *m_currentNamespace;
    };
}

#endif // LYRIC_COMPILER_TYPENAME_HANDLER_H
