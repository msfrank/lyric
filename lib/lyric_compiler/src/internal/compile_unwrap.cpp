
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/extension_callable.h>
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

    // if an identifier is specified, then declare an alias of the target but with unwrap type
    if (walker.hasAttr(lyric_parser::kLyricAstIdentifier)) {
        std::string identifier;
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);
        TU_RETURN_IF_STATUS (block->declareAlias(identifier, targetRef, unwrapType));
    }

    // if there are no unwrap params, then we are done
    if (walker.numChildren() == 0)
        return {};

    // parse the unwrap list and declare locals for each param
    std::vector<lyric_common::TypeDef> tupleTypeArguments;
    std::vector<std::pair<std::string,lyric_assembler::DataReference>> unwrapRefs;
    for (int i = 0; i < walker.numChildren(); i++) {
        auto param = walker.getChild(i);
        moduleEntry.checkClassOrThrow(param, lyric_schema::kLyricAstParamClass);

        lyric_parser::NodeWalker typeNode;
        moduleEntry.parseAttrOrThrow(param, lyric_parser::kLyricAstTypeOffset, typeNode);
        lyric_typing::TypeSpec paramSpec;
        TU_ASSIGN_OR_RETURN (paramSpec, typeSystem->parseAssignable(block, typeNode));
        lyric_common::TypeDef paramType;
        TU_ASSIGN_OR_RETURN (paramType, typeSystem->resolveAssignable(block, paramSpec));
        tupleTypeArguments.push_back(paramType);

        std::string paramName;
        moduleEntry.parseAttrOrThrow(param, lyric_parser::kLyricAstIdentifier, paramName);
        lyric_assembler::DataReference ref;
        TU_ASSIGN_OR_RETURN (ref, block->declareVariable(
            paramName, lyric_object::AccessType::Private, paramType, /* isVariable= */ true));
        unwrapRefs.emplace_back(paramName, ref);
    }

    // resolve the instance implementing unwrap() for the specified unwrap type and tuple type
    lyric_assembler::DataReference instanceRef;
    TU_ASSIGN_OR_RETURN (instanceRef, resolve_unwrap_instance(block, unwrapType,
        tupleTypeArguments, moduleEntry));
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

    lyric_assembler::CallableInvoker extensionInvoker;
    TU_RETURN_IF_NOT_OK (impl->prepareExtension(kUnwrapExtensionName, instanceRef, extensionInvoker));

    auto instanceTypeArguments = instanceRef.typeDef.getConcreteArguments();
    std::vector<lyric_common::TypeDef> callsiteTypeArguments(
        instanceTypeArguments.begin(), instanceTypeArguments.end());
    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(extensionInvoker, callsiteTypeArguments));

    TU_RETURN_IF_NOT_OK (block->load(targetRef));
    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(unwrapType));

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
        TU_RETURN_IF_NOT_OK (block->load(tupleRef));

        // drop the previous result from the stack
        auto unwrapRef = unwrapRefs[i];
        TU_RETURN_IF_NOT_OK (block->store(unwrapRef.second));
    }

    auto *blockCode = block->blockCode();
    auto *fragment = blockCode->rootFragment();

    // pop the tuple off the stack
    TU_RETURN_IF_NOT_OK (fragment->popValue());

    return {};
}
