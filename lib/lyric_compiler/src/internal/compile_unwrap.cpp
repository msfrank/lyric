
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/code_builder.h>
#include <lyric_assembler/extension_invoker.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_compiler/internal/compile_unwrap.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/callsite_reifier.h>
#include <lyric_typing/member_reifier.h>
#include <tempo_utils/log_stream.h>

static tempo_utils::Result<lyric_assembler::DataReference>
resolve_unwrap_instance(
    lyric_assembler::BlockHandle *block,
    const lyric_common::TypeDef &unwrapType,
    const std::vector<lyric_common::TypeDef> &tupleTypeArguments,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    auto *state = moduleEntry.getState();

    // construct the tuple type
    int tupleTypeArity = tupleTypeArguments.size();
    auto fundamentalTuple = state->fundamentalCache()->getTupleUrl(tupleTypeArity);
    if (!fundamentalTuple.isValid())
        block->throwAssemblerInvariant("tuple arity is too large");
    auto tupleType = lyric_common::TypeDef::forConcrete(fundamentalTuple, tupleTypeArguments);

    // resolve the instance implementing unwrap() for the specified unwrap type and tuple type
    auto fundamentalUnwrap = state->fundamentalCache()->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Unwrap);
    auto concreteReceiverType = lyric_common::TypeDef::forConcrete(
        fundamentalUnwrap, {unwrapType, tupleType});
    auto resolveConcreteInstanceResult = block->resolveImpl(concreteReceiverType,
        lyric_assembler::ResolveMode::kNoStatusIfMissing);
    if (resolveConcreteInstanceResult.isStatus())
        return resolveConcreteInstanceResult.getStatus();

    // if the result is a valid symbol, then return symbol binding
    auto concreteInstanceUrl = resolveConcreteInstanceResult.getResult();
    if (concreteInstanceUrl.isValid())
        return lyric_assembler::DataReference(concreteInstanceUrl,
            concreteReceiverType, lyric_assembler::ReferenceType::Value);

    // resolve the instance for a generic unwrap type with a concrete tuple type
    lyric_assembler::AbstractSymbol *unwrapSym;
    TU_ASSIGN_OR_RETURN (unwrapSym, state->symbolCache()->getOrImportSymbol(unwrapType.getConcreteUrl()));
    auto genericUnwrapType = unwrapSym->getAssignableType();
    auto genericConcreteReceiverType = lyric_common::TypeDef::forConcrete(
        fundamentalUnwrap, {genericUnwrapType, tupleType});
    auto resolveGenericConcreteInstanceResult = block->resolveImpl(genericConcreteReceiverType,
        lyric_assembler::ResolveMode::kNoStatusIfMissing);
    if (resolveGenericConcreteInstanceResult.isStatus())
        return resolveGenericConcreteInstanceResult.getStatus();

    // if the result is a valid symbol, then return symbol binding
    auto genericConcreteInstanceUrl = resolveGenericConcreteInstanceResult.getResult();
    if (genericConcreteInstanceUrl.isValid())
        return lyric_assembler::DataReference(genericConcreteInstanceUrl,
            genericConcreteReceiverType, lyric_assembler::ReferenceType::Value);

    // resolve the instance for a generic unwrap type with a generic tuple type of the same arity
    auto genericUnwrapTypeArguments = genericUnwrapType.getConcreteArguments();
    int genericTupleTypeArity = genericUnwrapTypeArguments.size();
    auto genericTupleUrl = state->fundamentalCache()->getTupleUrl(genericTupleTypeArity);
    if (!genericTupleUrl.isValid())
        block->throwAssemblerInvariant("tuple arity is too large");
    auto genericTupleType = lyric_common::TypeDef::forConcrete(
        genericTupleUrl, std::vector<lyric_common::TypeDef>(
            genericUnwrapTypeArguments.begin(), genericUnwrapTypeArguments.end()));
    auto genericGenericReceiverType = lyric_common::TypeDef::forConcrete(
        fundamentalUnwrap, {genericUnwrapType, genericTupleType});

    auto resolveGenericGenericInstanceResult = block->resolveImpl(genericGenericReceiverType);
    if (resolveGenericGenericInstanceResult.isStatus())
        return resolveGenericGenericInstanceResult.getStatus();

    return lyric_assembler::DataReference(resolveGenericGenericInstanceResult.getResult(),
        genericGenericReceiverType, lyric_assembler::ReferenceType::Value);
}

tempo_utils::Status
lyric_compiler::internal::compile_unwrap(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    const lyric_common::TypeDef &unwrapType,
    const lyric_assembler::DataReference &targetRef,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();
    auto *state = moduleEntry.getState();
    auto *symbolCache = state->symbolCache();

    tempo_utils::Status status;

    // if an identifier is specified, then declare an alias of the target but with unwrap type
    if (walker.hasAttr(lyric_parser::kLyricAstIdentifier)) {
        std::string identifier;
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);
        auto declareAliasResult = block->declareAlias(identifier, targetRef, unwrapType);
        if (declareAliasResult.isStatus())
            return declareAliasResult.getStatus();
    }

    // if there are no unwrap params, then we are done
    if (walker.numChildren() == 0)
        return CompilerStatus::ok();

    // parse the unwrap list and declare locals for each param
    std::vector<lyric_common::TypeDef> tupleTypeArguments;
    std::vector<std::pair<std::string,lyric_assembler::DataReference>> unwrapRefs;
    for (int i = 0; i < walker.numChildren(); i++) {
        auto param = walker.getChild(i);
        moduleEntry.checkClassOrThrow(param, lyric_schema::kLyricAstParamClass);

        tu_uint32 typeOffset;
        moduleEntry.parseAttrOrThrow(param, lyric_parser::kLyricAstTypeOffset, typeOffset);
        auto type = param.getNodeAtOffset(typeOffset);
        auto resolveParamTypeResult = typeSystem->resolveAssignable(block, type);
        if (resolveParamTypeResult.isStatus())
            return resolveParamTypeResult.getStatus();
        auto paramType = resolveParamTypeResult.getResult();
        tupleTypeArguments.push_back(paramType);

        std::string paramName;
        moduleEntry.parseAttrOrThrow(param, lyric_parser::kLyricAstIdentifier, paramName);
        auto declareParamResult = block->declareVariable(
            paramName, paramType, lyric_parser::BindingType::VARIABLE);
        if (declareParamResult.isStatus())
            return declareParamResult.getStatus();
        unwrapRefs.emplace_back(paramName, declareParamResult.getResult());
    }

    // resolve the instance implementing unwrap() for the specified unwrap type and tuple type
    auto resolveUnwrapInstanceResult = resolve_unwrap_instance(block, unwrapType,
        tupleTypeArguments, moduleEntry);
    if (resolveUnwrapInstanceResult.isStatus())
        return resolveUnwrapInstanceResult.getStatus();
    auto instanceRef = resolveUnwrapInstanceResult.getResult();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(instanceRef.symbolUrl));
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::INSTANCE)
        block->throwAssemblerInvariant("invalid instance symbol {}", instanceRef.symbolUrl.toString());
    auto *instanceSymbol = cast_symbol_to_instance(symbol);

    // resolve Unwrap magnet
    auto *impl = instanceSymbol->getImpl(instanceRef.typeDef);
    if (impl == nullptr)
        return block->logAndContinue(CompilerCondition::kMissingImpl,
            tempo_tracing::LogSeverity::kError,
            "missing impl for {}", instanceRef.typeDef.toString());

    auto extensionOption = impl->getExtension(kUnwrapExtensionName);
    if (extensionOption.isEmpty())
        return block->logAndContinue(CompilerCondition::kMissingAction,
            tempo_tracing::LogSeverity::kError,
            "missing extension {} for impl {}", kUnwrapExtensionName, instanceRef.typeDef.toString());
    auto extension = extensionOption.getValue();

    auto extensionUrl = extension.methodCall;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(extensionUrl));
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::CALL)
        block->throwAssemblerInvariant("invalid call symbol {}", extensionUrl.toString());
    auto *extensionCall = cast_symbol_to_call(symbol);

    lyric_assembler::ExtensionInvoker extensionInvoker;
    if (extensionCall->isInline()) {
        extensionInvoker = lyric_assembler::ExtensionInvoker(extensionCall, extensionCall->callProc());
    } else if (extensionCall->isBound()) {
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(instanceRef.typeDef.getConcreteUrl()));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
            block->throwAssemblerInvariant("invalid concept symbol {}", instanceRef.typeDef.getConcreteUrl().toString());
        auto *conceptSymbol = cast_symbol_to_concept(symbol);

        auto resolveActionResult = conceptSymbol->getAction(kUnwrapExtensionName);
        if (resolveActionResult.isEmpty())
            block->throwAssemblerInvariant("missing action {} for concept symbol {}",
                kUnwrapExtensionName, instanceRef.typeDef.getConcreteUrl().toString());
        auto action = resolveActionResult.getValue();
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(action.methodAction));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::ACTION)
            block->throwAssemblerInvariant("invalid action symbol {}", action.methodAction.toString());
        auto *actionSymbol = cast_symbol_to_action(symbol);

        instanceSymbol->touch();

        extensionInvoker = lyric_assembler::ExtensionInvoker(conceptSymbol, actionSymbol, instanceRef);
    } else {
        block->throwAssemblerInvariant("invalid extension call {}", extensionUrl.toString());
    }

    // call unwrap() on the target, putting the tuple on the top of the stack
    auto callsiteTypeArguments = instanceRef.typeDef.getConcreteArguments();
    lyric_typing::CallsiteReifier reifier(extensionInvoker.getParameters(), extensionInvoker.getRest(),
        extensionInvoker.getTemplateUrl(), extensionInvoker.getTemplateParameters(),
        std::vector<lyric_common::TypeDef>(callsiteTypeArguments.begin(), callsiteTypeArguments.end()),
        typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize());
    status = block->load(targetRef);
    if (!status.isOk())
        return status;
    status = reifier.reifyNextArgument(unwrapType);
    if (!status.isOk())
        return status;
    auto invokeUnwrapResult = extensionInvoker.invoke(block, reifier);
    if (invokeUnwrapResult.isStatus())
        return invokeUnwrapResult.getStatus();
    auto tupleType = invokeUnwrapResult.getResult();
    auto tupleUrl = tupleType.getConcreteUrl();

    // store each tuple element in a local variable
    for (tu_uint32 i = 0; i < tupleTypeArguments.size(); i++) {
        auto tupleMember = absl::StrCat("t", i);

        lyric_assembler::AbstractSymbol *receiver;
        TU_ASSIGN_OR_RETURN (receiver, symbolCache->getOrImportSymbol(tupleUrl));

        lyric_assembler::DataReference tupleRef;
        switch (receiver->getSymbolType()) {
            case lyric_assembler::SymbolType::CLASS: {
                auto *classSymbol = cast_symbol_to_class(receiver);
                lyric_typing::MemberReifier memberReifier(typeSystem, tupleType, classSymbol->classTemplate());
                auto resolveMemberResult = classSymbol->resolveMember(tupleMember, memberReifier, tupleType);
                if (resolveMemberResult.isStatus())
                    return resolveMemberResult.getStatus();
                tupleRef = resolveMemberResult.getResult();
                break;
            }
            default:
                block->throwAssemblerInvariant("invalid receiver symbol {}", tupleUrl.toString());
        }

        // load member from tuple onto the top of the stack
        status = block->load(tupleRef);
        if (!status.isOk())
            return status;

        // drop the previous result from the stack
        auto unwrapRef = unwrapRefs[i];
        status = block->store(unwrapRef.second);
        if (!status.isOk())
            return status;
    }

    // pop the tuple off the stack
    status = block->blockCode()->popValue();
    if (!status.isOk())
        return status;

    return CompilerStatus::ok();
}
