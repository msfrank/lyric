
#include <absl/strings/str_join.h>
#include <unicode/ustring.h>

#include <ModuleParser.h>

#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_constant_ops.h>
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
    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstTrueClass, token);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitFalseLiteral(ModuleParser::FalseLiteralContext *ctx)
{
    auto *token = ctx->FalseKeyword()->getSymbol();
    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstFalseClass, token);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitNilLiteral(ModuleParser::NilLiteralContext *ctx)
{
    auto *token = ctx->NilKeyword()->getSymbol();
    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstNilClass, token);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitSymbolLiteral(ModuleParser::SymbolLiteralContext *ctx)
{
    auto *token = ctx->getStart();

    std::vector<std::string> parts;
    for (size_t i = 0; i < ctx->getRuleIndex(); i++) {
        if (ctx->Identifier(i) == nullptr)
            continue;
        auto part = ctx->Identifier(i)->getText();
        if (part.empty())
            m_state->throwSyntaxError(ctx->Identifier(i)->getSymbol(), "symbol literal part is empty");
        parts.push_back(part);
    }
    lyric_common::SymbolPath symbolPath(parts);

    if (parts.empty())
        m_state->throwSyntaxError(token, "symbol literal has no parts");

    auto *symbolPathAttr = m_state->appendAttrOrThrow(kLyricAstSymbolPath, symbolPath);

    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstSymbolRefClass, token);
    literalNode->putAttr(symbolPathAttr);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitDecimalInteger(ModuleParser::DecimalIntegerContext *ctx)
{
    auto *token = ctx->DecimalInteger()->getSymbol();

    auto value = token->getText();
    if (value.empty())
        m_state->throwSyntaxError(token, "invalid decimal integer");

    auto *literalValueAttr = m_state->appendAttrOrThrow(kLyricAstLiteralValue, value);
    auto *baseAttr = m_state->appendAttrOrThrow(kLyricAstBaseType, BaseType::DECIMAL);

    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstIntegerClass, token);
    literalNode->putAttr(literalValueAttr);
    literalNode->putAttr(baseAttr);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitHexInteger(ModuleParser::HexIntegerContext *ctx)
{
    auto *token = ctx->HexInteger()->getSymbol();

    auto src = token->getText();
    if (src.empty())
        m_state->throwSyntaxError(token, "invalid hex integer");

    auto value = std::string(src.cbegin() + 2, src.cend()); // strip the 0x, 0X prefix
    if (value.empty())
        m_state->throwSyntaxError(token, "invalid hex integer");

    auto *literalValueAttr = m_state->appendAttrOrThrow(kLyricAstLiteralValue, value);
    auto *baseAttr = m_state->appendAttrOrThrow(kLyricAstBaseType, BaseType::HEX);

    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstIntegerClass, token);
    literalNode->putAttr(literalValueAttr);
    literalNode->putAttr(baseAttr);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitOctalInteger(ModuleParser::OctalIntegerContext *ctx)
{
    auto *token = ctx->OctalInteger()->getSymbol();

    auto src = token->getText();
    if (src.empty())
        m_state->throwSyntaxError(token, "invalid octal integer");

    auto value = std::string(src.cbegin() + 1, src.cend()); // strip the 0 prefix
    if (value.empty())
        m_state->throwSyntaxError(token, "invalid octal integer");

    auto *literalValueAttr = m_state->appendAttrOrThrow(kLyricAstLiteralValue, value);
    auto *baseAttr = m_state->appendAttrOrThrow(kLyricAstBaseType, BaseType::OCTAL);

    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstIntegerClass, token);
    literalNode->putAttr(literalValueAttr);
    literalNode->putAttr(baseAttr);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitDecimalFixedFloat(ModuleParser::DecimalFixedFloatContext *ctx)
{
    auto *token = ctx->DecimalFixedFloat()->getSymbol();

    auto value = token->getText();
    if (value.empty())
        m_state->throwSyntaxError(token, "invalid decimal fixed float");

    auto *literalValueAttr = m_state->appendAttrOrThrow(kLyricAstLiteralValue, value);
    auto *baseAttr = m_state->appendAttrOrThrow(kLyricAstBaseType, BaseType::DECIMAL);
    auto *notationAttr = m_state->appendAttrOrThrow(kLyricAstNotationType, NotationType::FIXED);

    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstFloatClass, token);
    literalNode->putAttr(literalValueAttr);
    literalNode->putAttr(baseAttr);
    literalNode->putAttr(notationAttr);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitDecimalScientificFloat(ModuleParser::DecimalScientificFloatContext *ctx)
{
    auto *token = ctx->DecimalScientificFloat()->getSymbol();

    auto value = token->getText();
    if (value.empty())
        m_state->throwSyntaxError(token, "invalid decimal scientific float");

    auto *literalValueAttr = m_state->appendAttrOrThrow(kLyricAstLiteralValue, value);
    auto *baseAttr = m_state->appendAttrOrThrow(kLyricAstBaseType, BaseType::DECIMAL);
    auto *notationAttr = m_state->appendAttrOrThrow(kLyricAstNotationType, NotationType::SCIENTIFIC);

    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstFloatClass, token);
    literalNode->putAttr(literalValueAttr);
    literalNode->putAttr(baseAttr);
    literalNode->putAttr(notationAttr);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitHexFloat(ModuleParser::HexFloatContext *ctx)
{
    auto *token = ctx->HexFloat()->getSymbol();

    auto src = token->getText();
    if (src.empty())
        m_state->throwSyntaxError(token, "invalid hex float");

    auto value = std::string(src.cbegin() + 2, src.cend()); // strip the 0x, 0X prefix
    if (value.empty())
        m_state->throwSyntaxError(token, "invalid hex float");

    auto *literalValueAttr = m_state->appendAttrOrThrow(kLyricAstLiteralValue, value);
    auto *baseAttr = m_state->appendAttrOrThrow(kLyricAstBaseType, BaseType::HEX);
    auto *notationAttr = m_state->appendAttrOrThrow(kLyricAstNotationType, NotationType::FIXED);

    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstFloatClass, token);
    literalNode->putAttr(literalValueAttr);
    literalNode->putAttr(baseAttr);
    literalNode->putAttr(notationAttr);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitCharLiteral(ModuleParser::CharLiteralContext *ctx)
{
    auto *token = ctx->CharLiteral()->getSymbol();

    auto src = token->getText();
    if (src.empty())
        m_state->throwSyntaxError(token, "invalid char literal");

    auto value = std::string(src.cbegin() + 1, src.cend() - 1); // remove quotes
    if (value.empty())
        m_state->throwSyntaxError(token, "invalid char literal");

    auto *literalValueAttr = m_state->appendAttrOrThrow(kLyricAstLiteralValue, value);

    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCharClass, token);
    literalNode->putAttr(literalValueAttr);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitStringLiteral(ModuleParser::StringLiteralContext *ctx)
{
    auto *token = ctx->StringLiteral()->getSymbol();

    auto src = token->getText();
    if (src.empty())
        m_state->throwSyntaxError(token, "invalid string literal");

    auto value = std::string(src.cbegin() + 1, src.cend() - 1); // remove quotes

    auto *literalValueAttr = m_state->appendAttrOrThrow(kLyricAstLiteralValue, value);

    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstStringClass, token);
    literalNode->putAttr(literalValueAttr);
    m_state->pushNode(literalNode);
}

void
lyric_parser::internal::ModuleConstantOps::exitUrlLiteral(ModuleParser::UrlLiteralContext *ctx)
{
    auto *token = ctx->UrlLiteral()->getSymbol();

    auto src = token->getText();
    if (src.empty())
        m_state->throwSyntaxError(token, "invalid url literal");

    auto value = std::string(src.cbegin() + 1, src.cend() - 1); // remove quotes

    auto *literalValueAttr = m_state->appendAttrOrThrow(kLyricAstLiteralValue, value);

    auto *literalNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstUrlClass, token);
    literalNode->putAttr(literalValueAttr);
    m_state->pushNode(literalNode);
}