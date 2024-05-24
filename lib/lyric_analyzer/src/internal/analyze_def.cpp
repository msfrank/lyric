
#include <lyric_analyzer/internal/analyze_def.h>
#include <lyric_analyzer/internal/analyze_node.h>
#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

tempo_utils::Status
lyric_analyzer::internal::analyze_def(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT(walker.isValid());
    entryPoint.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstDefClass, 2);

    // get function name
    std::string identifier;
    entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    auto *typeSystem = entryPoint.getTypeSystem();

    // get function return type
    tu_uint32 typeOffset;
    entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    auto compileTypeResult = typeSystem->parseAssignable(block, type);
    if (compileTypeResult.isStatus())
        return compileTypeResult.getStatus();
    auto returnSpec = compileTypeResult.getResult();

    // compile the parameter list
    auto pack = walker.getChild(0);
    auto parsePackResult = typeSystem->parsePack(block, pack);
    if (parsePackResult.isStatus())
        return parsePackResult.getStatus();
    auto packSpec = parsePackResult.getResult();

    // if function is generic, then compile the template parameter list
    lyric_assembler::TemplateSpec templateSpec;
    if (walker.hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        tu_uint32 genericOffset;
        entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstGenericOffset, genericOffset);
        auto generic = walker.getNodeAtOffset(genericOffset);
        auto resolveTemplateResult = typeSystem->resolveTemplate(block, generic);
        if (resolveTemplateResult.isStatus())
            return resolveTemplateResult.getStatus();
        templateSpec = resolveTemplateResult.getResult();
    }

    // declare the function
    auto declareFunctionResult = block->declareFunction(identifier,
        packSpec.parameterSpec, packSpec.restSpec, packSpec.ctxSpec,
        returnSpec, lyric_object::AccessType::Public, templateSpec.templateParameters, true);
    if (declareFunctionResult.isStatus())
        return declareFunctionResult.getStatus();
    auto functionUrl = declareFunctionResult.getResult();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, block->blockState()->symbolCache()->getOrImportSymbol(functionUrl));
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::CALL)
        block->throwAssemblerInvariant("invalid call symbol {}", functionUrl.toString());
    auto *call = cast_symbol_to_call(symbol);

    // analyze the function body
    auto body = walker.getChild(1);
    auto *proc = call->callProc();
    return analyze_block(proc->procBlock(), body, entryPoint);
}