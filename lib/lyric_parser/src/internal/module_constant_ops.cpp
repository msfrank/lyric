
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
    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstTrueClass, location);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitFalseLiteral(ModuleParser::FalseLiteralContext *ctx)
{
    auto *token = ctx->FalseKeyword()->getSymbol();
    auto location = get_token_location(token);
    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstFalseClass, location);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitUndefLiteral(ModuleParser::UndefLiteralContext *ctx)
{
    auto *token = ctx->UndefKeyword()->getSymbol();
    auto location = get_token_location(token);
    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstUndefClass, location);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitNilLiteral(ModuleParser::NilLiteralContext *ctx)
{
    auto *token = ctx->NilKeyword()->getSymbol();
    auto location = get_token_location(token);
    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstNilClass, location);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitSymbolLiteral(ModuleParser::SymbolLiteralContext *ctx)
{
    std::vector<std::string> parts;
    for (size_t i = 0; i < ctx->getRuleIndex(); i++) {
        if (ctx->Identifier(i) == nullptr)
            continue;
        auto *id = ctx->Identifier(i);
        auto location = get_token_location(id->getSymbol());
        auto part = id->getText();
        if (part.empty())
            m_state->throwSyntaxError(location, "symbol literal part is empty");
        parts.push_back(part);
    }
    lyric_common::SymbolPath symbolPath(parts);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    if (parts.empty())
        m_state->throwSyntaxError(location, "symbol literal has no parts");

    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstSymbolRefClass, location);
    literalNode->putAttr(kLyricAstSymbolPath, symbolPath);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitDecimalInteger(ModuleParser::DecimalIntegerContext *ctx)
{
    auto *token = ctx->DecimalInteger()->getSymbol();
    auto location = get_token_location(token);

    auto value = token->getText();
    if (value.empty())
        m_state->throwSyntaxError(location, "invalid decimal integer");

    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstIntegerClass, location);
    literalNode->putAttr(kLyricAstLiteralValue, value);
    literalNode->putAttr(kLyricAstBaseType, BaseType::Decimal);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitHexInteger(ModuleParser::HexIntegerContext *ctx)
{
    auto *token = ctx->HexInteger()->getSymbol();
    auto location = get_token_location(token);

    auto src = token->getText();
    if (src.empty())
        m_state->throwSyntaxError(location, "invalid hex integer");

    auto value = std::string(src.cbegin() + 2, src.cend()); // strip the 0x, 0X prefix
    if (value.empty())
        m_state->throwSyntaxError(location, "invalid hex integer");

    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstIntegerClass, location);
    literalNode->putAttr(kLyricAstLiteralValue, value);
    literalNode->putAttr(kLyricAstBaseType, BaseType::Hex);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitOctalInteger(ModuleParser::OctalIntegerContext *ctx)
{
    auto *token = ctx->OctalInteger()->getSymbol();
    auto location = get_token_location(token);

    auto src = token->getText();
    if (src.empty())
        m_state->throwSyntaxError(location, "invalid octal integer");

    auto value = std::string(src.cbegin() + 1, src.cend()); // strip the 0 prefix
    if (value.empty())
        m_state->throwSyntaxError(location, "invalid octal integer");

    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstIntegerClass, location);
    literalNode->putAttr(kLyricAstLiteralValue, value);
    literalNode->putAttr(kLyricAstBaseType, BaseType::Octal);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitDecimalFixedFloat(ModuleParser::DecimalFixedFloatContext *ctx)
{
    auto *token = ctx->DecimalFixedFloat()->getSymbol();
    auto location = get_token_location(token);

    auto value = token->getText();
    if (value.empty())
        m_state->throwSyntaxError(location, "invalid decimal fixed float");

    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstFloatClass, location);
    literalNode->putAttr(kLyricAstLiteralValue, value);
    literalNode->putAttr(kLyricAstBaseType, BaseType::Decimal);
    literalNode->putAttr(kLyricAstNotationType, NotationType::Fixed);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitDecimalScientificFloat(ModuleParser::DecimalScientificFloatContext *ctx)
{
    auto *token = ctx->DecimalScientificFloat()->getSymbol();
    auto location = get_token_location(token);

    auto value = token->getText();
    if (value.empty())
        m_state->throwSyntaxError(location, "invalid decimal scientific float");

    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstFloatClass, location);
    literalNode->putAttr(kLyricAstLiteralValue, value);
    literalNode->putAttr(kLyricAstBaseType, BaseType::Decimal);
    literalNode->putAttr(kLyricAstNotationType, NotationType::Scientific);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitHexFloat(ModuleParser::HexFloatContext *ctx)
{
    auto *token = ctx->HexFloat()->getSymbol();
    auto location = get_token_location(token);

    auto src = token->getText();
    if (src.empty())
        m_state->throwSyntaxError(location, "invalid hex float");

    auto value = std::string(src.cbegin() + 2, src.cend()); // strip the 0x, 0X prefix
    if (value.empty())
        m_state->throwSyntaxError(location, "invalid hex float");

    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstFloatClass, location);
    literalNode->putAttr(kLyricAstLiteralValue, value);
    literalNode->putAttr(kLyricAstBaseType, BaseType::Hex);
    literalNode->putAttr(kLyricAstNotationType, NotationType::Fixed);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitCharLiteral(ModuleParser::CharLiteralContext *ctx)
{
    auto *token = ctx->CharLiteral()->getSymbol();
    auto location = get_token_location(token);

    auto src = token->getText();
    if (src.empty())
        m_state->throwSyntaxError(location, "invalid char literal");

    auto value = std::string(src.cbegin() + 1, src.cend() - 1); // remove quotes
    if (value.empty())
        m_state->throwSyntaxError(location, "invalid char literal");

    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCharClass, location);
    literalNode->putAttr(kLyricAstLiteralValue, value);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitStringLiteral(ModuleParser::StringLiteralContext *ctx)
{
    auto *token = ctx->StringLiteral()->getSymbol();
    auto location = get_token_location(token);

    auto src = token->getText();
    if (src.empty())
        m_state->throwSyntaxError(location, "invalid string literal");

    auto value = std::string(src.cbegin() + 1, src.cend() - 1); // remove quotes

    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstStringClass, location);
    literalNode->putAttr(kLyricAstLiteralValue, value);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitUrlLiteral(ModuleParser::UrlLiteralContext *ctx)
{
    auto *token = ctx->UrlLiteral()->getSymbol();
    auto location = get_token_location(token);

    auto src = token->getText();
    if (src.empty())
        m_state->throwSyntaxError(location, "invalid url literal");

    auto value = std::string(src.cbegin() + 1, src.cend() - 1); // remove quotes

    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstUrlClass, location);
    literalNode->putAttr(kLyricAstLiteralValue, value);
    m_state->pushNode(literalNode);
}