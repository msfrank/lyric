
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/literal_cache.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_compiler/internal/compile_constant.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/parse_literal.h>
#include <lyric_typing/callsite_reifier.h>

static inline tempo_utils::Result<lyric_common::TypeDef>
compile_nil_constant(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_RETURN_IF_NOT_OK (block->blockCode()->loadNil());
    return block->blockState()->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Nil);
}

static inline tempo_utils::Result<lyric_common::TypeDef>
compile_undef_constant(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_RETURN_IF_NOT_OK (block->blockCode()->loadUndef());
    return block->blockState()->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Undef);
}

static inline tempo_utils::Result<lyric_common::TypeDef>
compile_true_constant(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_RETURN_IF_NOT_OK (block->blockCode()->loadBool(true));
    return block->blockState()->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);
}

static inline tempo_utils::Result<lyric_common::TypeDef>
compile_false_constant(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_RETURN_IF_NOT_OK (block->blockCode()->loadBool(false));
    return block->blockState()->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);
}

static inline tempo_utils::Result<lyric_common::TypeDef>
compile_char_constant(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (walker.isValid());

    std::string literalValue;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstLiteralValue, literalValue);

    UChar32 chr;
    TU_ASSIGN_OR_RETURN (chr, lyric_parser::parse_char_literal(literalValue));
    TU_RETURN_IF_NOT_OK (block->blockCode()->loadChar(chr));
    return block->blockState()->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Char);
}

static inline tempo_utils::Result<lyric_common::TypeDef>
compile_integer_constant(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (walker.isValid());

    std::string literalValue;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstLiteralValue, literalValue);
    lyric_parser::BaseType base;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstBaseType, base);

    tu_int64 i64;
    TU_ASSIGN_OR_RETURN (i64, lyric_parser::parse_integer_literal(literalValue, base));
    TU_RETURN_IF_NOT_OK (block->blockCode()->loadInt(i64));
    return block->blockState()->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
}

static inline tempo_utils::Result<lyric_common::TypeDef>
compile_float_constant(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (walker.isValid());

    std::string literalValue;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstLiteralValue, literalValue);
    lyric_parser::BaseType base;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstBaseType, base);
    lyric_parser::NotationType notation;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstNotationType, notation);

    double dbl;
    TU_ASSIGN_OR_RETURN (dbl, lyric_parser::parse_float_literal(literalValue, base, notation));
    TU_RETURN_IF_NOT_OK (block->blockCode()->loadFloat(dbl));
    return block->blockState()->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Float);
}

inline tempo_utils::Result<lyric_common::TypeDef>
compile_string_constant(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (walker.isValid());
    auto *state = moduleEntry.getState();
    auto *fundamentalCache = state->fundamentalCache();
    auto *literalCache = state->literalCache();

    std::string literalValue;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstLiteralValue, literalValue);

    std::string str;
    TU_ASSIGN_OR_RETURN (str, lyric_parser::parse_string_literal(literalValue));
    lyric_assembler::LiteralAddress address;
    TU_ASSIGN_OR_RETURN (address, literalCache->makeLiteralUtf8(str));

    auto *code = block->blockCode();
    TU_RETURN_IF_NOT_OK (code->loadString(address));

    auto fundamentalString = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::String);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, state->symbolCache()->getOrImportSymbol(fundamentalString));
    symbol->touch();

    return fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::String);
}

inline tempo_utils::Result<lyric_common::TypeDef>
compile_url_constant(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (walker.isValid());
    auto *state = moduleEntry.getState();
    auto *fundamentalCache = state->fundamentalCache();
    auto *literalCache = state->literalCache();

    std::string literalValue;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstLiteralValue, literalValue);

    std::string str;
    TU_ASSIGN_OR_RETURN (str, lyric_parser::parse_string_literal(literalValue));
    lyric_assembler::LiteralAddress address;
    TU_ASSIGN_OR_RETURN (address, literalCache->makeLiteralUtf8(str));

    auto *code = block->blockCode();
    TU_RETURN_IF_NOT_OK (code->loadUrl(address));

    auto fundamentalUrl = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Url);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, state->symbolCache()->getOrImportSymbol(fundamentalUrl));
    symbol->touch();

    return fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Url);
}

inline tempo_utils::Result<lyric_common::TypeDef>
compile_symbol_constant(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT (walker.isValid());

    lyric_common::SymbolPath symbolPath;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstSymbolPath, symbolPath);
    if (!symbolPath.isValid())
        return block->logAndContinue(lyric_compiler::CompilerCondition::kInvalidSymbol,
            tempo_tracing::LogSeverity::kError,
            "invalid symbol constant {}", symbolPath.toString());

    lyric_assembler::DataReference constantRef;

    for (const auto &identifier : symbolPath.getPath()) {
        TU_ASSIGN_OR_RETURN (constantRef, block->resolveReference(identifier));

        if (constantRef.referenceType != lyric_assembler::ReferenceType::Descriptor)
            return block->logAndContinue(lyric_compiler::CompilerCondition::kMissingSymbol,
                tempo_tracing::LogSeverity::kError,
                "missing symbol {}", symbolPath.toString());

        lyric_assembler::AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, block->blockState()->symbolCache()->getOrImportSymbol(constantRef.symbolUrl));

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
                return block->logAndContinue(lyric_compiler::CompilerCondition::kInvalidSymbol,
                    tempo_tracing::LogSeverity::kError,
                    "invalid symbol constant {}", constantRef.symbolUrl.toString());
        }
    }

    TU_RETURN_IF_NOT_OK (block->load(constantRef));
    return constantRef.typeDef;
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::internal::compile_constant(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    lyric_schema::LyricAstId nodeId{};
    moduleEntry.parseIdOrThrow(walker, lyric_schema::kLyricAstVocabulary, nodeId);

    switch (nodeId) {

        case lyric_schema::LyricAstId::Nil:
            return compile_nil_constant(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::Undef:
            return compile_undef_constant(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::True:
            return compile_true_constant(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::False:
            return compile_false_constant(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::Char:
            return compile_char_constant(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::Integer:
            return compile_integer_constant(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::Float:
            return compile_float_constant(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::String:
            return compile_string_constant(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::Url:
            return compile_url_constant(block, walker, moduleEntry);
        case lyric_schema::LyricAstId::SymbolRef:
            return compile_symbol_constant(block, walker, moduleEntry);

        default:
            moduleEntry.throwCompilerInvariant("failed to parse constant; unexpected parser node");
    }
}
