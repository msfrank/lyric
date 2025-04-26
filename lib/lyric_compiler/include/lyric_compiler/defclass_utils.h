#ifndef LYRIC_COMPILER_DEFCLASS_UTILS_H
#define LYRIC_COMPILER_DEFCLASS_UTILS_H

#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_parser/archetype_node.h>
#include <lyric_typing/type_system.h>

#include "defclass_handler.h"
#include "impl_handler.h"
#include "member_handler.h"
#include "method_handler.h"

namespace lyric_compiler {

    tempo_utils::Result<lyric_assembler::CallSymbol *>
    declare_class_default_init(
        const DefClass *defclass,
        lyric_assembler::ClassSymbol *classSymbol,
        const std::string &allocatorTrap,
        lyric_assembler::SymbolCache *symbolCache,
        lyric_typing::TypeSystem *typeSystem);

    tempo_utils::Result<lyric_assembler::CallSymbol *>
    declare_class_init(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::ClassSymbol *classSymbol,
        const std::string &allocatorTrap,
        lyric_typing::TypeSystem *typeSystem);

    tempo_utils::Result<Member>
    declare_class_member(
        const lyric_parser::ArchetypeNode *node,
        bool isVariable,
        lyric_assembler::ClassSymbol *classSymbol,
        lyric_typing::TypeSystem *typeSystem);

    tempo_utils::Result<Method>
    declare_class_method(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::ClassSymbol *classSymbol,
        lyric_typing::TypeSystem *typeSystem);

    tempo_utils::Result<Impl>
    declare_class_impl(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::ClassSymbol *classSymbol,
        lyric_typing::TypeSystem *typeSystem);
}

#endif // LYRIC_COMPILER_DEFCLASS_UTILS_H
