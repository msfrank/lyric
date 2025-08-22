#ifndef LYRIC_COMPILER_DEFINSTANCE_UTILS_H
#define LYRIC_COMPILER_DEFINSTANCE_UTILS_H

#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_parser/archetype_node.h>
#include <lyric_typing/type_system.h>

#include "definstance_handler.h"
#include "impl_handler.h"
#include "member_handler.h"
#include "method_handler.h"

namespace lyric_compiler {

    tempo_utils::Result<lyric_assembler::CallSymbol *>
    declare_instance_init(
        lyric_assembler::InstanceSymbol *instanceSymbol,
        const std::string &allocatorTrap);

    tempo_utils::Status
    define_instance_default_init(
        const DefInstance *definstance,
        lyric_assembler::SymbolCache *symbolCache,
        lyric_typing::TypeSystem *typeSystem);

    tempo_utils::Result<Member>
    declare_instance_member(
        const lyric_parser::ArchetypeNode *node,
        bool isVariable,
        lyric_assembler::InstanceSymbol *instanceSymbol,
        lyric_typing::TypeSystem *typeSystem);

    tempo_utils::Result<Method>
    declare_instance_method(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::InstanceSymbol *instanceSymbol,
        lyric_typing::TypeSystem *typeSystem);

    tempo_utils::Result<Impl>
    declare_instance_impl(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::InstanceSymbol *instanceSymbol,
        lyric_typing::TypeSystem *typeSystem);
}

#endif // LYRIC_COMPILER_DEFINSTANCE_UTILS_H
