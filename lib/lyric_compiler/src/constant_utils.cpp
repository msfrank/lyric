
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
lyric_compiler::compile_nil(
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
{
    auto *fundamentalCache = block->blockState()->fundamentalCache();
    TU_RETURN_IF_NOT_OK (block->blockCode()->loadNil());
    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Nil));
}

tempo_utils::Status
lyric_compiler::compile_undef(
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
{
    auto *fundamentalCache = block->blockState()->fundamentalCache();
    TU_RETURN_IF_NOT_OK (block->blockCode()->loadUndef());
    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Undef));
}

tempo_utils::Status
lyric_compiler::compile_true(
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
{
    auto *fundamentalCache = block->blockState()->fundamentalCache();
    TU_RETURN_IF_NOT_OK (block->blockCode()->loadBool(true));
    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool));
}

tempo_utils::Status
lyric_compiler::compile_false(
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
{
    auto *fundamentalCache = block->blockState()->fundamentalCache();
    TU_RETURN_IF_NOT_OK (block->blockCode()->loadBool(false));
    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool));
}

tempo_utils::Status
lyric_compiler::compile_char(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
{
    auto *fundamentalCache = block->blockState()->fundamentalCache();

    std::string literalValue;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue));

    UChar32 chr;
    TU_ASSIGN_OR_RETURN (chr, lyric_parser::parse_char_literal(literalValue));
    TU_RETURN_IF_NOT_OK (block->blockCode()->loadChar(chr));
    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Char));
}

tempo_utils::Status
lyric_compiler::compile_integer(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
{
    auto *fundamentalCache = block->blockState()->fundamentalCache();

    std::string literalValue;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue));
    lyric_parser::BaseType base;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstBaseType, base));

    tu_int64 i64;
    TU_ASSIGN_OR_RETURN (i64, lyric_parser::parse_integer_literal(literalValue, base));
    TU_RETURN_IF_NOT_OK (block->blockCode()->loadInt(i64));
    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int));
}

tempo_utils::Status
lyric_compiler::compile_float(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
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
    TU_RETURN_IF_NOT_OK (block->blockCode()->loadFloat(dbl));
    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Float));
}

tempo_utils::Status
lyric_compiler::compile_string(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
{
    auto *state = block->blockState();
    auto *fundamentalCache = state->fundamentalCache();
    auto *literalCache = state->literalCache();
    auto *symbolCache = state->symbolCache();

    std::string literalValue;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue));

    std::string str;
    TU_ASSIGN_OR_RETURN (str, lyric_parser::parse_string_literal(literalValue));
    lyric_assembler::LiteralAddress address;
    TU_ASSIGN_OR_RETURN (address, literalCache->makeLiteralUtf8(str));

    auto *code = block->blockCode();
    TU_RETURN_IF_NOT_OK (code->loadString(address));

    auto fundamentalString = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::String);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(fundamentalString));
    symbol->touch();

    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::String));
}

tempo_utils::Status
lyric_compiler::compile_url(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
{
    auto *state = block->blockState();
    auto *fundamentalCache = state->fundamentalCache();
    auto *literalCache = state->literalCache();
    auto *symbolCache = state->symbolCache();

    std::string literalValue;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue));

    std::string str;
    TU_ASSIGN_OR_RETURN (str, lyric_parser::parse_string_literal(literalValue));
    lyric_assembler::LiteralAddress address;
    TU_ASSIGN_OR_RETURN (address, literalCache->makeLiteralUtf8(str));

    auto *code = block->blockCode();
    TU_RETURN_IF_NOT_OK (code->loadUrl(address));

    auto fundamentalUrl = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Url);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(fundamentalUrl));
    symbol->touch();

    return driver->pushResult(fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Url));
}

tempo_utils::Status
lyric_compiler::compile_symbol(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
{
    auto *state = block->blockState();
    auto *symbolCache = state->symbolCache();

    lyric_common::SymbolPath symbolPath;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath));
    if (!symbolPath.isValid())
        return block->logAndContinue(CompilerCondition::kInvalidSymbol,
            tempo_tracing::LogSeverity::kError,
            "invalid symbol constant {}", symbolPath.toString());

    lyric_assembler::DataReference constantRef;

    for (const auto &identifier : symbolPath.getPath()) {
        TU_ASSIGN_OR_RETURN (constantRef, block->resolveReference(identifier));

        if (constantRef.referenceType != lyric_assembler::ReferenceType::Descriptor)
            return block->logAndContinue(CompilerCondition::kMissingSymbol,
                tempo_tracing::LogSeverity::kError,
                "missing symbol {}", symbolPath.toString());

        lyric_assembler::AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(constantRef.symbolUrl));

        switch (symbol->getSymbolType()) {
            case lyric_assembler::SymbolType::CLASS: {
                block = cast_symbol_to_class(symbol)->classBlock();
                break;
            }
            case lyric_assembler::SymbolType::CONCEPT: {
                block = cast_symbol_to_concept(symbol)->conceptBlock();
                break;
            }
            case lyric_assembler::SymbolType::INSTANCE: {
                block = cast_symbol_to_instance(symbol)->instanceBlock();
                break;
            }
            case lyric_assembler::SymbolType::STRUCT: {
                block = cast_symbol_to_struct(symbol)->structBlock();
                break;
            }
            case lyric_assembler::SymbolType::ENUM: {
                block = cast_symbol_to_enum(symbol)->enumBlock();
                break;
            }
            default:
                return block->logAndContinue(CompilerCondition::kInvalidSymbol,
                    tempo_tracing::LogSeverity::kError,
                    "invalid symbol constant {}", constantRef.symbolUrl.toString());
        }
    }

    TU_RETURN_IF_NOT_OK (block->load(constantRef));

    return driver->pushResult(constantRef.typeDef);
}