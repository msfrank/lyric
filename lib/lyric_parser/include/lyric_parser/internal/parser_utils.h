#ifndef LYRIC_PARSER_INTERNAL_PARSER_UTILS_H
#define LYRIC_PARSER_INTERNAL_PARSER_UTILS_H

#include <antlr4-runtime.h>

#include <ModuleLexer.h>
#include <ModuleParser.h>
#include <ModuleParserBaseListener.h>

#include <lyric_common/symbol_path.h>

#include "ModuleParser.h"
#include "../archetype_node.h"
#include "../parser_types.h"

namespace lyric_parser::internal {

    ParseLocation get_token_location(const antlr4::Token *token);

    lyric_common::SymbolPath make_symbol_path(ModuleParser::SymbolPathContext *ctx);

    ArchetypeNode *make_SType_node(ArchetypeState *state, ModuleParser::SimpleTypeContext *ctx);
    ArchetypeNode *make_PType_node(ArchetypeState *state, ModuleParser::ParametricTypeContext *ctx);
    ArchetypeNode *make_UType_node(ArchetypeState *state, ModuleParser::UnionTypeContext *ctx);
    ArchetypeNode *make_IType_node(ArchetypeState *state, ModuleParser::IntersectionTypeContext *ctx);
    ArchetypeNode *make_SType_or_PType_node(ArchetypeState *state, ModuleParser::SingularTypeContext *ctx);
    ArchetypeNode *make_Type_node(ArchetypeState *state, ModuleParser::AssignableTypeContext *ctx);
    ArchetypeNode *make_TypeArguments_node(ArchetypeState *state, ModuleParser::TypeArgumentsContext *ctx);
    ArchetypeNode *make_Generic_node(
        ArchetypeState *state,
        ModuleParser::PlaceholderSpecContext *pctx,
        ModuleParser::ConstraintSpecContext *cctx = nullptr);

    bool identifier_is_hidden(std::string_view identifier);
}

#endif // LYRIC_PARSER_INTERNAL_PARSER_UTILS_H
