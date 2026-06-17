
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/literal_cache.h>
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
    TU_RETURN_IF_NOT_OK (fragment->immediateC32(chr));
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
    lyric_common::NumericBase base;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstBaseType, base));

    tu_int64 i64;
    TU_ASSIGN_OR_RETURN (i64, lyric_common::parse_I64(literalValue, base));
    TU_RETURN_IF_NOT_OK (fragment->immediateI64(i64));
    TU_LOG_VV << "immediate I64 " << i64;
    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::I64));
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
    lyric_common::NumericBase base;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstBaseType, base));

    bool scientific = false;
    if (node->hasAttr(lyric_parser::kLyricAstIsScientific)) {
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsScientific, scientific));
    }

    double f64;
    TU_ASSIGN_OR_RETURN (f64, lyric_common::parse_F64(literalValue, base, scientific));
    TU_RETURN_IF_NOT_OK (fragment->immediateF64(f64));
    TU_LOG_VV << "immediate F64 " << f64;
    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::F64));
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
lyric_compiler::constant_raw(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    auto *state = block->blockState();
    auto *fundamentalCache = state->fundamentalCache();

    std::string literalValue;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue));

    std::span bytes((const tu_uint8 *) literalValue.data(), literalValue.size());
    TU_RETURN_IF_NOT_OK (fragment->loadBytes(bytes));

    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bytes));
}