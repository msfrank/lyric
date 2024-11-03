#ifndef LYRIC_COMPILER_DEFCONCEPT_UTILS_H
#define LYRIC_COMPILER_DEFCONCEPT_UTILS_H

#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_parser/archetype_node.h>
#include <lyric_typing/type_system.h>

#include "action_handler.h"
#include "defconcept_handler.h"
#include "impl_handler.h"

namespace lyric_compiler {

    tempo_utils::Result<Action>
    declare_concept_action(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::ConceptSymbol *conceptSymbol,
        lyric_typing::TypeSystem *typeSystem);

    tempo_utils::Result<Impl>
    declare_concept_impl(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::ConceptSymbol *conceptSymbol,
        lyric_typing::TypeSystem *typeSystem);
}

#endif // LYRIC_COMPILER_DEFCONCEPT_UTILS_H
