
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/internal/compile_initializer.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_compiler/internal/compile_var.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

tempo_utils::Status
lyric_compiler::internal::compile_var(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT(walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();
    auto *state = moduleEntry.getState();

    moduleEntry.checkClassAndChildCountOrThrow(walker, lyric_schema::kLyricAstVarClass, 1);

    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    lyric_common::TypeDef varType;
    if (walker.hasAttr(lyric_parser::kLyricAstTypeOffset)) {
        tu_uint32 typeOffset;
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
        auto assignedType = walker.getNodeAtOffset(typeOffset);
        auto resolveVarTypeResult = typeSystem->resolveAssignable(block, assignedType);
        if (resolveVarTypeResult.isStatus())
            return resolveVarTypeResult.getStatus();
        varType = resolveVarTypeResult.getResult();
    }

    // if this is a root block, then the val is a static
    if (block->isRoot()) {
        if (!varType.isValid())
            return block->logAndContinue(CompilerCondition::kSyntaxError,
                tempo_tracing::LogSeverity::kError,
                "missing type for static val {}", identifier);

        auto declareStaticResult = block->declareStatic(identifier, varType, lyric_parser::BindingType::VARIABLE);
        if (declareStaticResult.isStatus())
            return declareStaticResult.getStatus();
        auto var = declareStaticResult.getResult();

        auto *sym = state->symbolCache()->getSymbol(var.symbol);
        if (sym == nullptr)
            state->throwAssemblerInvariant("missing static symbol {}", var.symbol.toString());
        if (sym->getSymbolType() != lyric_assembler::SymbolType::STATIC)
            state->throwAssemblerInvariant("invalid static symbol {}", var.symbol.toString());
        auto *staticSymbol = cast_symbol_to_static(sym);

        return compile_static_initializer(staticSymbol, walker.getChild(0), moduleEntry);
    }

    // expression is expected to push the value onto the stack
    auto expr = walker.getChild(0);
    auto exprResult = compile_expression(block, expr, moduleEntry);
    if (exprResult.isStatus())
        return exprResult.getStatus();
    auto resultType = exprResult.getResult();

    // if var type is not explicitly declared, then use the derived type
    if (!varType.isValid())
        varType = exprResult.getResult();

    // var type was explicitly declared and is disjoint from rvalue type
    if (!typeSystem->isAssignable(varType, resultType))
        return block->logAndContinue(CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "rvalue type {} is incompatible with var type {}", resultType.toString(), varType.toString());

    auto declareVariableResult = block->declareVariable(identifier, varType, lyric_parser::BindingType::VARIABLE);
    if (declareVariableResult.isStatus())
        return declareVariableResult.getStatus();
    auto var = declareVariableResult.getResult();

    return block->store(var);
}
