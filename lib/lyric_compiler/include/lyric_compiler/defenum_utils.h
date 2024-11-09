#ifndef LYRIC_COMPILER_DEFENUM_UTILS_H
#define LYRIC_COMPILER_DEFENUM_UTILS_H

#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_parser/archetype_node.h>
#include <lyric_typing/type_system.h>

#include "defenum_handler.h"
#include "impl_handler.h"
#include "member_handler.h"
#include "method_handler.h"

namespace lyric_compiler {

    tempo_utils::Result<lyric_assembler::CallSymbol *>
    declare_enum_default_init(
        const DefEnum *defenum,
        lyric_assembler::EnumSymbol *enumSymbol,
        lyric_assembler::SymbolCache *symbolCache,
        lyric_typing::TypeSystem *typeSystem);

    tempo_utils::Result<lyric_assembler::CallSymbol *>
    declare_enum_init(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::EnumSymbol *enumSymbol,
        lyric_typing::TypeSystem *typeSystem);

    tempo_utils::Result<Member>
    declare_enum_member(
        const lyric_parser::ArchetypeNode *node,
        bool isVariable,
        lyric_assembler::EnumSymbol *enumSymbol,
        lyric_typing::TypeSystem *typeSystem);

    tempo_utils::Result<Method>
    declare_enum_method(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::EnumSymbol *enumSymbol,
        lyric_typing::TypeSystem *typeSystem);

    tempo_utils::Result<Impl>
    declare_enum_impl(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::EnumSymbol *enumSymbol,
        lyric_typing::TypeSystem *typeSystem);

    tempo_utils::Result<lyric_assembler::EnumSymbol *>
    declare_enum_case(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::EnumSymbol *enumSymbol,
        lyric_assembler::BlockHandle *block,
        lyric_typing::TypeSystem *typeSystem);
}

#endif // LYRIC_COMPILER_DEFENUM_UTILS_H
