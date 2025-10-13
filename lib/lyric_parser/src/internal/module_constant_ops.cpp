
#include <absl/strings/str_join.h>

#include <ModuleParser.h>

#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_constant_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

#include "lyric_parser/internal/semantic_exception.h"

lyric_parser::internal::ModuleConstantOps::ModuleConstantOps(ModuleArchetype *listener)
    : BaseOps(listener)
{
}

void
lyric_parser::internal::ModuleConstantOps::parseTrueLiteral(ModuleParser::TrueLiteralContext *ctx)
{
    auto *token = ctx->TrueKeyword()->getSymbol();
    auto location = get_token_location(token);
    auto *state = getState();

    if (hasError())
        return;

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, state->appendNode(lyric_schema::kLyricAstTrueClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::parseFalseLiteral(ModuleParser::FalseLiteralContext *ctx)
{
    auto *state = getState();
    auto *token = ctx->FalseKeyword()->getSymbol();
    auto location = get_token_location(token);

    if (hasError())
        return;

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, state->appendNode(lyric_schema::kLyricAstFalseClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::parseUndefLiteral(ModuleParser::UndefLiteralContext *ctx)
{
    auto *state = getState();
    auto *token = ctx->UndefKeyword()->getSymbol();
    auto location = get_token_location(token);

    if (hasError())
        return;

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, state->appendNode(lyric_schema::kLyricAstUndefClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::parseNilLiteral(ModuleParser::NilLiteralContext *ctx)
{
    auto *state = getState();
    auto *token = ctx->NilKeyword()->getSymbol();
    auto location = get_token_location(token);

    if (hasError())
        return;

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, state->appendNode(lyric_schema::kLyricAstNilClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::parseDecimalInteger(ModuleParser::DecimalIntegerContext *ctx)
{
    auto *state = getState();
    auto location = get_token_location(ctx->getStart());

    auto value = ctx->getText();
    if (value.empty()) {
        logErrorOrThrow(ctx->getStart(), "invalid decimal integer ''");
    }

    if (hasError())
        return;

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, state->appendNode(lyric_schema::kLyricAstIntegerClass, location));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstLiteralValue, value));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstBaseType, BaseType::Decimal));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstNotationType, NotationType::Integral));
    TU_RAISE_IF_NOT_OK (state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::parseHexInteger(ModuleParser::HexIntegerContext *ctx)
{
    auto *state = getState();
    auto location = get_token_location(ctx->getStart());

    std::string value;
    if (ctx->MinusOperator() != nullptr) {
        value.append("-");
    }
    value.append(ctx->HexInteger()->getText());

    if (value.empty()) {
        logErrorOrThrow(ctx->getStart(), "invalid hex integer ''");
    }

    if (hasError())
        return;

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, state->appendNode(lyric_schema::kLyricAstIntegerClass, location));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstLiteralValue, value));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstBaseType, BaseType::Hex));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstNotationType, NotationType::Integral));
    TU_RAISE_IF_NOT_OK (state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::parseOctalInteger(ModuleParser::OctalIntegerContext *ctx)
{
    auto *state = getState();
    auto location = get_token_location(ctx->getStart());

    std::string value;
    if (ctx->MinusOperator() != nullptr) {
        value.append("-");
    }
    value.append(ctx->OctalInteger()->getText());

    if (value.empty()) {
        logErrorOrThrow(ctx->getStart(), "invalid octal integer ''");
    }

    if (hasError())
        return;

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, state->appendNode(lyric_schema::kLyricAstIntegerClass, location));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstLiteralValue, value));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstBaseType, BaseType::Octal));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstNotationType, NotationType::Integral));
    TU_RAISE_IF_NOT_OK (state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::parseDecimalFixedFloat(ModuleParser::DecimalFixedFloatContext *ctx)
{
    auto *state = getState();
    auto location = get_token_location(ctx->getStart());

    auto value = ctx->getText();
    if (value.empty()) {
        logErrorOrThrow(ctx->getStart(), "invalid decimal fixed float ''");
    }

    if (hasError())
        return;

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, state->appendNode(lyric_schema::kLyricAstFloatClass, location));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstLiteralValue, value));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstBaseType, BaseType::Decimal));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstNotationType, NotationType::Fixed));
    TU_RAISE_IF_NOT_OK (state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::parseDecimalScientificFloat(ModuleParser::DecimalScientificFloatContext *ctx)
{
    auto *state = getState();
    auto location = get_token_location(ctx->getStart());

    auto value = ctx->getText();
    if (value.empty()) {
        logErrorOrThrow(ctx->getStart(), "invalid decimal scientific float ''");
    }

    if (hasError())
        return;

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, state->appendNode(lyric_schema::kLyricAstFloatClass, location));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstLiteralValue, value));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstBaseType, BaseType::Decimal));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstNotationType, NotationType::Scientific));
    TU_RAISE_IF_NOT_OK (state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::parseHexFloat(ModuleParser::HexFloatContext *ctx)
{
    auto *state = getState();
    auto location = get_token_location(ctx->getStart());

    std::string value;
    if (ctx->MinusOperator() != nullptr) {
        value.append("-");
    }

    antlr4::tree::TerminalNode *term;
    if (ctx->HexFloatLeadingPeriod()) {
        term = ctx->HexFloatLeadingPeriod();
    } else if (ctx->HexFloatTrailingPeriod()) {
        term = ctx->HexFloatTrailingPeriod();
    } else if (ctx->HexFloatNoPeriod()) {
        term = ctx->HexFloatNoPeriod();
    } else {
        throw SemanticException(ctx->getStart(), "invalid hex float ''");
    }
    value.append(term->getText());

    if (value.empty()) {
        logErrorOrThrow(ctx->getStart(), "invalid hex float '{}'", ctx->getText());
    }

    if (hasError())
        return;

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, state->appendNode(lyric_schema::kLyricAstFloatClass, location));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstLiteralValue, value));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstBaseType, BaseType::Hex));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstNotationType, NotationType::Fixed));
    TU_RAISE_IF_NOT_OK (state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::parseInvalidNumber(ModuleParser::InvalidNumberContext *ctx)
{
    auto value = ctx->getText();
    logErrorOrThrow(ctx->getStart(), "invalid number literal '{}'", value);
}

void
lyric_parser::internal::ModuleConstantOps::parseCharLiteral(ModuleParser::CharLiteralContext *ctx)
{
    auto *state = getState();
    auto *token = ctx->CharLiteral()->getSymbol();
    auto location = get_token_location(token);

    auto src = token->getText();
    if (src.empty()) {
        logErrorOrThrow(token, "invalid char literal ''");
        return;
    }

    auto value = std::string(src.cbegin() + 1, src.cend() - 1); // remove quotes
    if (value.empty()) {
        logErrorOrThrow(token, "invalid char literal '{}'", src);
        return;
    }

    if (hasError())
        return;

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, state->appendNode(lyric_schema::kLyricAstCharClass, location));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstLiteralValue, value));
    TU_RAISE_IF_NOT_OK (state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::parseStringLiteral(ModuleParser::StringLiteralContext *ctx)
{
    auto *state = getState();
    auto *token = ctx->StringLiteral()->getSymbol();
    auto location = get_token_location(token);

    auto src = token->getText();
    if (src.empty()) {
        throw SemanticException(token, "invalid string literal ''");
    }

    auto value = std::string(src.cbegin() + 1, src.cend() - 1); // remove quotes

    if (hasError())
        return;

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, state->appendNode(lyric_schema::kLyricAstStringClass, location));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstLiteralValue, value));
    TU_RAISE_IF_NOT_OK (state->pushNode(literalNode));
}

void
lyric_parser::internal::ModuleConstantOps::parseUrlLiteral(ModuleParser::UrlLiteralContext *ctx)
{
    auto *state = getState();
    auto *token = ctx->UrlLiteral()->getSymbol();
    auto location = get_token_location(token);

    auto src = token->getText();
    if (src.empty()) {
        throw SemanticException(token, "invalid url literal ''");
    }

    auto value = std::string(src.cbegin() + 1, src.cend() - 1); // remove quotes

    if (hasError())
        return;

    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, state->appendNode(lyric_schema::kLyricAstUrlClass, location));
    TU_RAISE_IF_NOT_OK (literalNode->putAttr(kLyricAstLiteralValue, value));
    TU_RAISE_IF_NOT_OK (state->pushNode(literalNode));
}