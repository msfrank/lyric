#ifndef LYRIC_COMPILER_ALIAS_UTILS_H
#define LYRIC_COMPILER_ALIAS_UTILS_H

#include <lyric_assembler/binding_symbol.h>
#include <lyric_parser/archetype_node.h>
#include <lyric_typing/type_system.h>

namespace lyric_compiler {

    tempo_utils::Result<lyric_assembler::BindingSymbol *> declare_alias(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::BlockHandle *block,
        lyric_typing::TypeSystem *typeSystem);

    tempo_utils::Result<lyric_assembler::BindingSymbol *> declare_alias(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::NamespaceSymbol *currentNamespace,
        lyric_assembler::BlockHandle *block,
        lyric_typing::TypeSystem *typeSystem);
}

#endif // LYRIC_COMPILER_ALIAS_UTILS_H
