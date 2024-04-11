
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/internal/compile_def.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_compiler/internal/compile_initializer.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

tempo_utils::Status
lyric_compiler::internal::compile_def(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry,
    lyric_assembler::CallSymbol **callSymbolPtr)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT(walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();

    moduleEntry.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstDefClass, 2);

    // get function name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // get function return type
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    auto parseAssignableResult = typeSystem->parseAssignable(block, type);
    if (parseAssignableResult.isStatus())
        return parseAssignableResult.getStatus();
    auto returnSpec = parseAssignableResult.getResult();

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
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstGenericOffset, genericOffset);
        auto generic = walker.getNodeAtOffset(genericOffset);
        auto resolveTemplateResult = typeSystem->resolveTemplate(block, generic);
        if (resolveTemplateResult.isStatus())
            return resolveTemplateResult.getStatus();
        templateSpec = resolveTemplateResult.getResult();
    }

    // compile initializers
    absl::flat_hash_map<std::string,lyric_common::SymbolUrl> initializers;
    for (const auto &p : packSpec.parameterSpec) {
        if (!p.init.isEmpty()) {
            auto compileInitializerResult = compile_default_initializer(
                block, p.name, templateSpec.templateParameters, p.type, p.init.getValue(), moduleEntry);
            if (compileInitializerResult.isStatus())
                return compileInitializerResult.getStatus();
            initializers[p.name] = compileInitializerResult.getResult();
        }
    }

    // declare the function
    auto declareFunctionResult = block->declareFunction(identifier,
        packSpec.parameterSpec, packSpec.restSpec, packSpec.ctxSpec,
        returnSpec, lyric_object::AccessType::Public, templateSpec.templateParameters);
    if (declareFunctionResult.isStatus())
        return declareFunctionResult.getStatus();
    auto functionUrl = declareFunctionResult.getResult();
    auto *sym = block->blockState()->symbolCache()->getSymbol(functionUrl);
    if (sym == nullptr)
        block->throwAssemblerInvariant("missing call symbol");
    if (sym->getSymbolType() != lyric_assembler::SymbolType::CALL)
        block->throwAssemblerInvariant("invalid call symbol");
    auto *call = cast_symbol_to_call(sym);

    // add initializers to the call
    for (const auto &entry : initializers) {
        call->putInitializer(entry.first, entry.second);
    }

    // compile the function body
    auto body = walker.getChild(1);
    auto *proc = call->callProc();
    auto compileBodyResult = compile_block(proc->procBlock(), body, moduleEntry);
    if (compileBodyResult.isStatus())
        return compileBodyResult.getStatus();
    auto bodyType = compileBodyResult.getResult();

    // add return instruction
    auto status = proc->procCode()->writeOpcode(lyric_object::Opcode::OP_RETURN);
    if (!status.isOk())
        return status;

    bool isReturnable;

    // validate that body returns the expected type
    TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(call->getReturnType(), bodyType));
    if (!isReturnable)
        return block->logAndContinue(body,
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "body does not match return type {}", call->getReturnType().toString());

    // validate that each exit returns the expected type
    for (const auto &exitType : call->listExitTypes()) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(call->getReturnType(), exitType));
        if (!isReturnable)
            return block->logAndContinue(body,
                lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "body does not match return type {}", call->getReturnType().toString());
    }

    if (callSymbolPtr != nullptr) {
        *callSymbolPtr = call;
    }

    // return control to the caller
    return CompilerStatus::ok();
}
