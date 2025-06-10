#ifndef LYRIC_PARSER_INTERNAL_PARSER_UTILS_H
#define LYRIC_PARSER_INTERNAL_PARSER_UTILS_H

#include <antlr4-runtime.h>

#include <ModuleLexer.h>
#include <ModuleParser.h>
#include <ModuleParserBaseListener.h>

#include "../archetype_node.h"
#include "../parser_types.h"

namespace lyric_parser::internal {

    ParseLocation get_token_location(const antlr4::Token *token);

    class SemanticException : public std::exception {
    public:
        SemanticException(const antlr4::Token *token, std::string_view message);
        const char *what() const noexcept override;
    private:
        ParseLocation m_location;
        std::string m_message;
    };

    void throw_semantic_exception(const antlr4::Token *token, std::string_view message);

    template<typename ...Args>
    void throw_semantic_exception(const antlr4::Token *token, fmt::string_view messageFmt, Args... messageArgs)
    {
        auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
        throw_semantic_exception(token, message);
    }


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
    AccessType parse_access_type(std::string_view identifier);
}

#endif // LYRIC_PARSER_INTERNAL_PARSER_UTILS_H
