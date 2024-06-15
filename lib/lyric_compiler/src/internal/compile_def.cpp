
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/internal/compile_def.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_compiler/internal/compile_initializer.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/typing_types.h>
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

    // parse the return type
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    lyric_parser::Assignable returnSpec;
    TU_ASSIGN_OR_RETURN (returnSpec, typeSystem->parseAssignable(block, type));

    // parse the parameter list
    auto pack = walker.getChild(0);
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(block, pack));

    // if function is generic, then parse the template parameter list
    lyric_typing::TemplateSpec templateSpec;
    if (walker.hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        tu_uint32 genericOffset;
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstGenericOffset, genericOffset);
        auto generic = walker.getNodeAtOffset(genericOffset);
        TU_ASSIGN_OR_RETURN (templateSpec, typeSystem->parseTemplate(block, generic));
    }

    // declare the function call
    lyric_assembler::CallSymbol *callSymbol;
    TU_ASSIGN_OR_RETURN (callSymbol, block->declareFunction(identifier,
        lyric_object::AccessType::Public, templateSpec.templateParameters));

    auto *resolver = callSymbol->callResolver();

    // resolve the parameter pack
    lyric_assembler::ParameterPack parameterPack;
    TU_ASSIGN_OR_RETURN (parameterPack, typeSystem->resolvePack(resolver, packSpec));

    // resolve the return type
    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, typeSystem->resolveAssignable(resolver, returnSpec));

    // compile list parameter initializers
    for (const auto &p : parameterPack.listParameters) {
        auto entry = packSpec.initializers.find(p.name);
        if (entry != packSpec.initializers.cend()) {
            lyric_common::SymbolUrl initializerUrl;
            TU_ASSIGN_OR_RETURN (initializerUrl, compile_param_initializer(
                block, entry->second, p, templateSpec.templateParameters, moduleEntry));
            callSymbol->putInitializer(p.name, initializerUrl);
        }
    }

    // compile named parameter initializers
    for (const auto &p : parameterPack.namedParameters) {
        auto entry = packSpec.initializers.find(p.name);
        if (entry != packSpec.initializers.cend()) {
            lyric_common::SymbolUrl initializerUrl;
            TU_ASSIGN_OR_RETURN (initializerUrl, compile_param_initializer(
                block, entry->second, p, templateSpec.templateParameters, moduleEntry));
            callSymbol->putInitializer(p.name, initializerUrl);
        }
    }

    // compile the function body
    auto body = walker.getChild(1);
    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, returnType));
    lyric_common::TypeDef bodyType;
    TU_ASSIGN_OR_RETURN (bodyType, compile_block(procHandle->procBlock(), body, moduleEntry));

    // add return instruction
    TU_RETURN_IF_NOT_OK (procHandle->procCode()->writeOpcode(lyric_object::Opcode::OP_RETURN));

    // validate that body returns the expected type
    bool isReturnable;
    TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(returnType, bodyType));
    if (!isReturnable)
        return block->logAndContinue(body,
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "body does not match return type {}", returnType.toString());

    // validate that each exit returns the expected type
    for (auto it = procHandle->exitTypesBegin(); it != procHandle->exitTypesEnd(); it++) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(returnType, *it));
        if (!isReturnable)
            return block->logAndContinue(body,
                lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "body does not match return type {}", returnType.toString());
    }

    if (callSymbolPtr != nullptr) {
        *callSymbolPtr = callSymbol;
    }

    return {};
}
