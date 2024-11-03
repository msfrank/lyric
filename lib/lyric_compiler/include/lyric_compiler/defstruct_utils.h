#ifndef LYRIC_COMPILER_DEFSTRUCT_UTILS_H
#define LYRIC_COMPILER_DEFSTRUCT_UTILS_H

#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_parser/archetype_node.h>
#include <lyric_typing/type_system.h>

#include "defstruct_handler.h"
#include "impl_handler.h"
#include "member_handler.h"
#include "method_handler.h"

namespace lyric_compiler {

    tempo_utils::Result<lyric_assembler::CallSymbol *>
    declare_struct_default_init(
        const DefStruct *defstruct,
        lyric_assembler::StructSymbol *structSymbol,
        lyric_assembler::SymbolCache *symbolCache,
        lyric_typing::TypeSystem *typeSystem);

    tempo_utils::Result<lyric_assembler::CallSymbol *>
    declare_struct_init(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::StructSymbol *structSymbol,
        lyric_typing::TypeSystem *typeSystem);

    tempo_utils::Status default_initialize_struct_members(
        lyric_assembler::StructSymbol *structSymbol,
        lyric_assembler::CallSymbol *ctorSymbol,
        lyric_assembler::SymbolCache *symbolCache,
        lyric_typing::TypeSystem *typeSystem);

    tempo_utils::Result<Member>
    declare_struct_member(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::StructSymbol *structSymbol,
        lyric_typing::TypeSystem *typeSystem);

    tempo_utils::Result<Method>
    declare_struct_method(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::StructSymbol *structSymbol,
        lyric_typing::TypeSystem *typeSystem);

    tempo_utils::Result<Impl>
    declare_struct_impl(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::StructSymbol *structSymbol,
        lyric_typing::TypeSystem *typeSystem);
}

#endif // LYRIC_COMPILER_DEFSTRUCT_UTILS_H
