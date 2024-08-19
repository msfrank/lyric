
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/internal/compiler_utils.h>
#include <lyric_compiler/internal/compile_defstatic.h>
#include <lyric_compiler/internal/compile_initializer.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

tempo_utils::Status
lyric_compiler::internal::compile_defstatic(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT(walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();
    auto *state = moduleEntry.getState();

    moduleEntry.checkClassAndChildCountOrThrow(walker, lyric_schema::kLyricAstDefStaticClass, 1);

    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    lyric_parser::AccessType access;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstAccessType, access);

    bool isVariable;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIsVariable, isVariable);

    lyric_parser::NodeWalker typeNode;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeNode);
    lyric_typing::TypeSpec staticSpec;
    TU_ASSIGN_OR_RETURN (staticSpec, typeSystem->parseAssignable(block, typeNode));
    lyric_common::TypeDef staticType;
    TU_ASSIGN_OR_RETURN (staticType, typeSystem->resolveAssignable(block, staticSpec));

    // if this is a root block, then the val is a static
    if (!staticType.isValid())
        return block->logAndContinue(CompilerCondition::kTypeError,
            tempo_tracing::LogSeverity::kError,
            "invalid type for static {}", identifier);

    lyric_assembler::DataReference ref;
    TU_ASSIGN_OR_RETURN (ref, block->declareStatic(
        identifier, lyric_compiler::internal::convert_access_type(access), staticType, isVariable));

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, state->symbolCache()->getOrImportSymbol(ref.symbolUrl));
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::STATIC)
        state->throwAssemblerInvariant("invalid static symbol {}", ref.symbolUrl.toString());
    auto *staticSymbol = cast_symbol_to_static(symbol);

    return compile_static_initializer(staticSymbol, walker.getChild(0), moduleEntry);
}
