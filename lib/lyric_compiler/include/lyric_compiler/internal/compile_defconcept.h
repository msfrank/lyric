#ifndef LYRIC_COMPILER_COMPILE_DEFCONCEPT_H
#define LYRIC_COMPILER_COMPILE_DEFCONCEPT_H

#include <lyric_assembler/block_handle.h>
#include <lyric_compiler/module_entry.h>

namespace lyric_compiler::internal {

    tempo_utils::Status compile_defconcept(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry,
        lyric_assembler::ConceptSymbol **conceptSymbolPtr = nullptr);
}

#endif // LYRIC_COMPILER_COMPILE_DEFCONCEPT_H