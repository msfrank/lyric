#ifndef LYRIC_COMPILER_INTERNAL_COMPILER_UTILS_H
#define LYRIC_COMPILER_INTERNAL_COMPILER_UTILS_H

#include <lyric_assembler/fundamental_cache.h>
#include <lyric_object/object_types.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>

namespace lyric_compiler::internal {

    lyric_object::AccessType convert_access_type(lyric_parser::AccessType access);

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
}

#endif // LYRIC_COMPILER_INTERNAL_COMPILER_UTILS_H
