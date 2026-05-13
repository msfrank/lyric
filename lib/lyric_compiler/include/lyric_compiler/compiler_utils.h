#ifndef LYRIC_COMPILER_COMPILER_UTILS_H
#define LYRIC_COMPILER_COMPILER_UTILS_H

#include <lyric_assembler/fundamental_cache.h>
#include <lyric_object/object_types.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/result.h>

#include "lyric_assembler/action_symbol.h"

namespace lyric_compiler {

    lyric_object::DeriveType convert_derive_type(lyric_parser::DeriveType derive);

    tempo_utils::Result<std::string> resolve_operator_action_name(lyric_schema::LyricAstId operatorClass);

    tempo_utils::Result<lyric_common::TypeDef> resolve_unary_operator_concept_type(
        const lyric_assembler::FundamentalCache *fundamentalCache,
        lyric_schema::LyricAstId operatorClass,
        const lyric_common::TypeDef &operand);

    tempo_utils::Result<lyric_common::TypeDef> resolve_binary_operator_concept_type(
        const lyric_assembler::FundamentalCache *fundamentalCache,
        lyric_schema::LyricAstId operatorClass,
        const lyric_common::TypeDef &operand1,
        const lyric_common::TypeDef &operand2);

    tempo_utils::Result<lyric_assembler::ActionSymbol *> resolve_operator_action(
        lyric_schema::LyricAstId operatorClass,
        lyric_assembler::FundamentalCache *fundamentalCache,
        lyric_assembler::SymbolCache *symbolCache);
}

#endif // LYRIC_COMPILER_COMPILER_UTILS_H
