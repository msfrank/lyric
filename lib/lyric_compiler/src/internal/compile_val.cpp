
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/internal/compile_initializer.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_compiler/internal/compile_val.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

tempo_utils::Status
lyric_compiler::internal::compile_val(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT(walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();
    auto *state = moduleEntry.getState();

    moduleEntry.checkClassAndChildCountOrThrow(walker, lyric_schema::kLyricAstValClass, 1);

    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    lyric_common::TypeDef valType;
    if (walker.hasAttr(lyric_parser::kLyricAstTypeOffset)) {
        lyric_parser::NodeWalker typeNode;
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeNode);
        lyric_typing::TypeSpec valSpec;
        TU_ASSIGN_OR_RETURN (valSpec, typeSystem->parseAssignable(block, typeNode));
        TU_ASSIGN_OR_RETURN (valType, typeSystem->resolveAssignable(block, valSpec));
    }

    // if this is a root block, then the val is a static
    if (block->isRoot()) {
        if (!valType.isValid())
            return block->logAndContinue(CompilerCondition::kSyntaxError,
                tempo_tracing::LogSeverity::kError,
                "missing type for static val {}", identifier);

        auto declareStaticResult = block->declareStatic(identifier, valType, lyric_parser::BindingType::VALUE);
        if (declareStaticResult.isStatus())
            return declareStaticResult.getStatus();
        auto ref = declareStaticResult.getResult();

        lyric_assembler::AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, state->symbolCache()->getOrImportSymbol(ref.symbolUrl));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::STATIC)
            state->throwAssemblerInvariant("invalid static symbol {}", ref.symbolUrl.toString());
        auto *staticSymbol = cast_symbol_to_static(symbol);

        return compile_static_initializer(staticSymbol, walker.getChild(0), moduleEntry);
    }

    // expression is expected to push the value onto the stack
    auto expr = walker.getChild(0);
    auto exprResult = compile_expression(block, expr, moduleEntry);
    if (exprResult.isStatus())
        return exprResult.getStatus();
    auto resultType = exprResult.getResult();

    // if val type is not explicitly declared, then use the derived type
    if (!valType.isValid())
        valType = resultType;

    // val type was explicitly declared and is disjoint from rvalue type
    bool isAssignable;
    TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(valType, resultType));
    if (!isAssignable)
        return block->logAndContinue(CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "rvalue type {} is incompatible with val type {}", resultType.toString(), valType.toString());

    auto declareVariableResult = block->declareVariable(identifier, valType, lyric_parser::BindingType::VALUE);
    if (declareVariableResult.isStatus())
        return declareVariableResult.getStatus();
    auto var = declareVariableResult.getResult();

    return block->store(var);
}
