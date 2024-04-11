
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
    auto status = block->blockCode()->loadNil();
    if (!status.isOk())
        return status;
    return block->blockState()->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Nil);
}

static inline tempo_utils::Result<lyric_common::TypeDef>
compile_true_constant(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    auto status = block->blockCode()->loadBool(true);
    if (!status.isOk())
        return status;
    return block->blockState()->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);
}

static inline tempo_utils::Result<lyric_common::TypeDef>
compile_false_constant(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    auto status = block->blockCode()->loadBool(false);
    if (!status.isOk())
        return status;
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

    auto compileCharResult = lyric_parser::parse_char_literal(literalValue);
    if (compileCharResult.isStatus())
        return compileCharResult.getStatus();
    auto status = block->blockCode()->loadChar(compileCharResult.getResult());
    if (!status.isOk())
        return status;
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

    auto compileIntegerResult = lyric_parser::parse_integer_literal(literalValue, base);
    if (compileIntegerResult.isStatus())
        return compileIntegerResult.getStatus();
    auto status = block->blockCode()->loadInt(compileIntegerResult.getResult());
    if (!status.isOk())
        return status;
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

    auto compileFloatResult = lyric_parser::parse_float_literal(literalValue, base, notation);
    if (compileFloatResult.isStatus())
        return compileFloatResult.getStatus();
    auto status = block->blockCode()->loadFloat(compileFloatResult.getResult());
    if (!status.isOk())
        return status;
    return block->blockState()->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Float);
}

static inline tempo_utils::Result<lyric_common::TypeDef>
new_struct_from_utf8_literal(
    lyric_typing::TypeSystem *typeSystem,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::LiteralAddress utf8,
    lyric_common::TypeDef structType)
{
    block->blockState()->typeCache()->touchType(structType);

    // resolve the struct
    auto structUrl = structType.getConcreteUrl();
    if (!block->blockState()->symbolCache()->hasSymbol(structUrl))
        return block->logAndContinue(lyric_compiler::CompilerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "missing symbol {}", structUrl.toString());
    auto *sym = block->blockState()->symbolCache()->getSymbol(structUrl);
    if (sym == nullptr)
        block->throwAssemblerInvariant("invalid struct");
    if (sym->getSymbolType() != lyric_assembler::SymbolType::STRUCT)
        block->throwAssemblerInvariant("invalid struct");
    auto *structSymbol = cast_symbol_to_struct(sym);

    structSymbol->touch();

    // push ctx argument values onto the stack
    auto resolveCtorResult = structSymbol->resolveCtor();
    if (resolveCtorResult.isStatus())
        return resolveCtorResult.getStatus();
    auto ctor = resolveCtorResult.getResult();

    lyric_typing::CallsiteReifier ctorReifier(ctor.getParameters(), ctor.getRest(),
        ctor.getTemplateUrl(), ctor.getTemplateParameters(),
        ctor.getTemplateArguments(), typeSystem);

    // push the utf16 literal onto the top of the stack as second arg
    TU_RETURN_IF_NOT_OK (block->blockCode()->loadLiteral(utf8));
    auto utf16Type = block->blockState()->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Utf8);
    TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(utf16Type));

    // invoke the ctor
    auto invokeNewResult = ctor.invokeNew(block, ctorReifier);
    if (invokeNewResult.isStatus())
        return invokeNewResult.getStatus();
    return invokeNewResult.getResult();
}

inline tempo_utils::Result<lyric_common::TypeDef>
compile_string_constant(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (walker.isValid());

    std::string literalValue;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstLiteralValue, literalValue);

    auto compileStringResult = lyric_parser::parse_string_literal(literalValue);
    if (compileStringResult.isStatus())
        return compileStringResult.getStatus();
    auto result = block->blockState()->literalCache()->makeLiteralUtf8(compileStringResult.getResult());
    if (result.isStatus())
        return result.getStatus();
    return new_struct_from_utf8_literal(moduleEntry.getTypeSystem(), block, result.getResult(),
        block->blockState()->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::String));
}

inline tempo_utils::Result<lyric_common::TypeDef>
compile_url_constant(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (walker.isValid());

    std::string literalValue;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstLiteralValue, literalValue);

    auto compileStringResult = lyric_parser::parse_string_literal(literalValue);
    if (compileStringResult.isStatus())
        return compileStringResult.getStatus();
    auto result = block->blockState()->literalCache()->makeLiteralUtf8(compileStringResult.getResult());
    if (result.isStatus())
        return result.getStatus();
    return new_struct_from_utf8_literal(moduleEntry.getTypeSystem(), block, result.getResult(),
        block->blockState()->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Url));
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
        auto resolveBindingResult = block->resolveReference(identifier);
        if (resolveBindingResult.isStatus())
            return tempo_utils::Result<lyric_common::TypeDef>(resolveBindingResult.getStatus());
        constantRef = resolveBindingResult.getResult();

        if (constantRef.referenceType != lyric_assembler::ReferenceType::Descriptor)
            return block->logAndContinue(lyric_compiler::CompilerCondition::kMissingSymbol,
                tempo_tracing::LogSeverity::kError,
                "missing symbol {}", symbolPath.toString());

        auto *symbol = block->blockState()->symbolCache()->getSymbol(constantRef.symbolUrl);
        if (symbol == nullptr)
            block->throwAssemblerInvariant("missing symbol {}", constantRef.symbolUrl.toString());

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

    auto status = block->load(constantRef);
    if (status.notOk())
        return status;
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
            break;
    }

    block->throwSyntaxError(walker, "invalid constant");
}
