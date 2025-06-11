
#include <absl/strings/str_join.h>
#include <unicode/ustring.h>

#include <ModuleParser.h>

#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_constant_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleConstantOps::ModuleConstantOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleConstantOps::exitTrueLiteral(ModuleParser::TrueLiteralContext *ctx)
{
    auto *token = ctx->TrueKeyword()->getSymbol();
    auto location = get_token_location(token);
    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, m_state->appendNode(lyric_schema::kLyricAstTrueClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::exitFalseLiteral(ModuleParser::FalseLiteralContext *ctx)
{
    auto *token = ctx->FalseKeyword()->getSymbol();
    auto location = get_token_location(token);
    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, m_state->appendNode(lyric_schema::kLyricAstFalseClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::exitUndefLiteral(ModuleParser::UndefLiteralContext *ctx)
{
    auto *token = ctx->UndefKeyword()->getSymbol();
    auto location = get_token_location(token);
    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, m_state->appendNode(lyric_schema::kLyricAstUndefClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::exitNilLiteral(ModuleParser::NilLiteralContext *ctx)
{
    auto *token = ctx->NilKeyword()->getSymbol();
    auto location = get_token_location(token);
    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, m_state->appendNode(lyric_schema::kLyricAstNilClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::exitDecimalInteger(ModuleParser::DecimalIntegerContext *ctx)
{
    auto *token = ctx->DecimalInteger()->getSymbol();
    auto location = get_token_location(token);

    auto value = token->getText();
    if (value.empty()) {
        throw_semantic_exception(token, "invalid decimal integer ''");
    }

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, m_state->appendNode(lyric_schema::kLyricAstIntegerClass, location));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstLiteralValue, value));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstBaseType, BaseType::Decimal));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstNotationType, NotationType::Integral));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::exitHexInteger(ModuleParser::HexIntegerContext *ctx)
{
    auto *token = ctx->HexInteger()->getSymbol();
    auto location = get_token_location(token);

    auto value = token->getText();
    if (value.empty()) {
        throw_semantic_exception(token, "invalid hex integer ''");
        TU_UNREACHABLE();
    }

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, m_state->appendNode(lyric_schema::kLyricAstIntegerClass, location));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstLiteralValue, value));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstBaseType, BaseType::Hex));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstNotationType, NotationType::Integral));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::exitOctalInteger(ModuleParser::OctalIntegerContext *ctx)
{
    auto *token = ctx->OctalInteger()->getSymbol();
    auto location = get_token_location(token);

    auto value = token->getText();
    if (value.empty()) {
        throw_semantic_exception(token, "invalid octal integer ''");
        TU_UNREACHABLE();
    }

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, m_state->appendNode(lyric_schema::kLyricAstIntegerClass, location));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstLiteralValue, value));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstBaseType, BaseType::Octal));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstNotationType, NotationType::Integral));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::exitDecimalFixedFloat(ModuleParser::DecimalFixedFloatContext *ctx)
{
    auto *token = ctx->DecimalFixedFloat()->getSymbol();
    auto location = get_token_location(token);

    auto value = token->getText();
    if (value.empty()) {
        throw_semantic_exception(token, "invalid decimal fixed float ''");
        TU_UNREACHABLE();
    }

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, m_state->appendNode(lyric_schema::kLyricAstFloatClass, location));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstLiteralValue, value));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstBaseType, BaseType::Decimal));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstNotationType, NotationType::Fixed));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::exitDecimalScientificFloat(ModuleParser::DecimalScientificFloatContext *ctx)
{
    auto *token = ctx->DecimalScientificFloat()->getSymbol();
    auto location = get_token_location(token);

    auto value = token->getText();
    if (value.empty()) {
        throw_semantic_exception(token, "invalid decimal scientific float ''");
        TU_UNREACHABLE();
    }

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, m_state->appendNode(lyric_schema::kLyricAstFloatClass, location));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstLiteralValue, value));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstBaseType, BaseType::Decimal));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstNotationType, NotationType::Scientific));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::exitHexFloat(ModuleParser::HexFloatContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto value = token->getText();
    if (value.empty()) {
        throw_semantic_exception(token, "invalid hex float ''");
        TU_UNREACHABLE();
    }

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, m_state->appendNode(lyric_schema::kLyricAstFloatClass, location));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstLiteralValue, value));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstBaseType, BaseType::Hex));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstNotationType, NotationType::Fixed));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::exitCharLiteral(ModuleParser::CharLiteralContext *ctx)
{
    auto *token = ctx->CharLiteral()->getSymbol();
    auto location = get_token_location(token);

    auto src = token->getText();
    if (src.empty()) {
        throw_semantic_exception(token, "invalid char literal ''");
        TU_UNREACHABLE();
    }

    auto value = std::string(src.cbegin() + 1, src.cend() - 1); // remove quotes
    if (value.empty()) {
        throw_semantic_exception(token, "invalid char literal ''");
        TU_UNREACHABLE();
    }

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, m_state->appendNode(lyric_schema::kLyricAstCharClass, location));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstLiteralValue, value));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::exitStringLiteral(ModuleParser::StringLiteralContext *ctx)
{
    auto *token = ctx->StringLiteral()->getSymbol();
    auto location = get_token_location(token);

    auto src = token->getText();
    if (src.empty()) {
        throw_semantic_exception(token, "invalid string literal ''");
        TU_UNREACHABLE();
    }

    auto value = std::string(src.cbegin() + 1, src.cend() - 1); // remove quotes

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, m_state->appendNode(lyric_schema::kLyricAstStringClass, location));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstLiteralValue, value));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::exitUrlLiteral(ModuleParser::UrlLiteralContext *ctx)
{
    auto *token = ctx->UrlLiteral()->getSymbol();
    auto location = get_token_location(token);

    auto src = token->getText();
    if (src.empty()) {
        throw_semantic_exception(token, "invalid url literal ''");
        TU_UNREACHABLE();
    }

    auto value = std::string(src.cbegin() + 1, src.cend() - 1); // remove quotes

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, m_state->appendNode(lyric_schema::kLyricAstUrlClass, location));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstLiteralValue, value));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(literalNode));
}