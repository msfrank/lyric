
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/literal_cache.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/constant_utils.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/parse_literal.h>

tempo_utils::Status
lyric_compiler::constant_nil(
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    auto *fundamentalCache = block->blockState()->fundamentalCache();
    TU_RETURN_IF_NOT_OK (fragment->immediateNil());
    TU_LOG_VV << "immediate nil";
    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Nil));
}

tempo_utils::Status
lyric_compiler::constant_undef(
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    auto *fundamentalCache = block->blockState()->fundamentalCache();
    TU_RETURN_IF_NOT_OK (fragment->immediateUndef());
    TU_LOG_VV << "immediate undef";
    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Undef));
}

tempo_utils::Status
lyric_compiler::constant_true(
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    auto *fundamentalCache = block->blockState()->fundamentalCache();
    TU_RETURN_IF_NOT_OK (fragment->immediateBool(true));
    TU_LOG_VV << "immediate true";
    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool));
}

tempo_utils::Status
lyric_compiler::constant_false(
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    auto *fundamentalCache = block->blockState()->fundamentalCache();
    TU_RETURN_IF_NOT_OK (fragment->immediateBool(false));
    TU_LOG_VV << "immediate false";
    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool));
}

tempo_utils::Status
lyric_compiler::constant_char(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    auto *fundamentalCache = block->blockState()->fundamentalCache();

    std::string literalValue;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue));

    char32_t chr;
    TU_ASSIGN_OR_RETURN (chr, lyric_parser::parse_char_literal(literalValue));
    TU_RETURN_IF_NOT_OK (fragment->immediateChar(chr));
    TU_LOG_VV << "immediate char '" << chr << "'";
    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Char));
}

tempo_utils::Status
lyric_compiler::constant_integer(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    auto *fundamentalCache = block->blockState()->fundamentalCache();

    std::string literalValue;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue));
    lyric_parser::BaseType base;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstBaseType, base));

    tu_int64 i64;
    TU_ASSIGN_OR_RETURN (i64, lyric_parser::parse_integer_literal(literalValue, base));
    TU_RETURN_IF_NOT_OK (fragment->immediateInt(i64));
    TU_LOG_VV << "immediate int " << i64;
    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int));
}

tempo_utils::Status
lyric_compiler::constant_float(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    auto *fundamentalCache = block->blockState()->fundamentalCache();

    std::string literalValue;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue));
    lyric_parser::BaseType base;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstBaseType, base));
    lyric_parser::NotationType notation;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstNotationType, notation));

    double dbl;
    TU_ASSIGN_OR_RETURN (dbl, lyric_parser::parse_float_literal(literalValue, base, notation));
    TU_RETURN_IF_NOT_OK (fragment->immediateFloat(dbl));
    TU_LOG_VV << "immediate float " << dbl;
    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Float));
}

tempo_utils::Status
lyric_compiler::constant_string(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    auto *state = block->blockState();
    auto *fundamentalCache = state->fundamentalCache();

    std::string literalValue;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue));

    std::string str;
    TU_ASSIGN_OR_RETURN (str, lyric_parser::parse_string_literal(literalValue));
    TU_RETURN_IF_NOT_OK (fragment->loadString(str));
    TU_LOG_VV << "immediate string \"" << str << "\"";

    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::String));
}

tempo_utils::Status
lyric_compiler::constant_url(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    auto *state = block->blockState();
    auto *fundamentalCache = state->fundamentalCache();

    std::string literalValue;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue));

    // TODO: implement lyric_parser::parse_url_literal
    std::string str;
    TU_ASSIGN_OR_RETURN (str, lyric_parser::parse_string_literal(literalValue));
    TU_RETURN_IF_NOT_OK (fragment->loadUrl(tempo_utils::Url::fromString(str)));
    TU_LOG_VV << "immediate url `" << str << "`";

    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Url));
}