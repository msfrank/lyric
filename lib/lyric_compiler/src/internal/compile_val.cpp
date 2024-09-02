
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/internal/compiler_utils.h>
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

    moduleEntry.checkClassAndChildCountOrThrow(walker, lyric_schema::kLyricAstValClass, 1);

    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    lyric_parser::AccessType access;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstAccessType, access);

    lyric_common::TypeDef valType;
    if (walker.hasAttr(lyric_parser::kLyricAstTypeOffset)) {
        lyric_parser::NodeWalker typeNode;
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeNode);
        lyric_typing::TypeSpec valSpec;
        TU_ASSIGN_OR_RETURN (valSpec, typeSystem->parseAssignable(block, typeNode));
        TU_ASSIGN_OR_RETURN (valType, typeSystem->resolveAssignable(block, valSpec));
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

//    // as a special case, if we are in the root block then declare the val in the entry call
//    if (block->isRoot()) {
//        block = moduleEntry.getEntry()->callProc()->procBlock();
//    }

    lyric_assembler::DataReference var;
    TU_ASSIGN_OR_RETURN (var, block->declareVariable(
        identifier, internal::convert_access_type(access), valType, /* isVariable= */ false));

    return block->store(var, /* initialStore= */ true);
}
