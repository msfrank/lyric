
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/impl_utils.h>
#include <lyric_compiler/unpack_handler.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_typing/callsite_reifier.h>
#include <lyric_typing/member_reifier.h>

lyric_compiler::UnpackHandler::UnpackHandler(
    const lyric_common::TypeDef &unwrapType,
    const lyric_assembler::DataReference &targetRef,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_fragment(fragment)
{
    TU_ASSERT (m_fragment != nullptr);
    m_unpack.unwrapType = unwrapType;
    TU_ASSERT (m_unpack.unwrapType.isValid());
    m_unpack.targetRef = targetRef;
    TU_ASSERT (m_unpack.targetRef.referenceType != lyric_assembler::ReferenceType::Invalid);
}

tempo_utils::Status lyric_compiler::UnpackHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    // if an identifier is specified, then declare an alias of the target but with unwrap type
    if (node->hasAttr(lyric_parser::kLyricAstIdentifier)) {
        std::string identifier;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
        TU_RETURN_IF_STATUS (block->declareAlias(identifier, m_unpack.targetRef, m_unpack.unwrapType));
    }

    for (int i = 0; i < node->numChildren(); i++) {
        auto param = std::make_unique<UnwrapParam>(&m_unpack, m_fragment, block, driver);
        ctx.appendChoice(std::move(param));
    }

    return {};
}

static tempo_utils::Result<lyric_assembler::ImplReference>
resolve_unwrap_impl(
    const lyric_common::TypeDef &unwrapType,
    const std::vector<lyric_common::TypeDef> &tupleTypeArguments,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
{
    auto *fundamentalCache = driver->getFundamentalCache();
    auto *symbolCache = driver->getSymbolCache();

    // construct the tuple type
    int tupleTypeArity = tupleTypeArguments.size();
    auto fundamentalTuple = fundamentalCache->getTupleUrl(tupleTypeArity);
    if (!fundamentalTuple.isValid())
        return block->logAndContinue(lyric_compiler::CompilerCondition::kCompilerInvariant,
            tempo_tracing::LogSeverity::kError,
            "tuple arity is too large");
    auto tupleType = lyric_common::TypeDef::forConcrete(fundamentalTuple, tupleTypeArguments);

    // resolve the instance implementing unwrap() for the specified unwrap type and tuple type
    auto fundamentalUnwrap = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Unwrap);
    auto concreteReceiverType = lyric_common::TypeDef::forConcrete(
        fundamentalUnwrap, {unwrapType, tupleType});

    // resolve the instance for a generic unwrap type with a concrete tuple type
    lyric_assembler::AbstractSymbol *unwrapSym;
    TU_ASSIGN_OR_RETURN (unwrapSym, symbolCache->getOrImportSymbol(unwrapType.getConcreteUrl()));
    auto genericUnwrapType = unwrapSym->getTypeDef();
    auto genericConcreteReceiverType = lyric_common::TypeDef::forConcrete(
        fundamentalUnwrap, {genericUnwrapType, tupleType});

    // resolve the instance for a generic unwrap type with a generic tuple type of the same arity
    auto genericUnwrapTypeArguments = genericUnwrapType.getConcreteArguments();
    int genericTupleTypeArity = genericUnwrapTypeArguments.size();
    auto genericTupleUrl = fundamentalCache->getTupleUrl(genericTupleTypeArity);
    if (!genericTupleUrl.isValid())
        return block->logAndContinue(lyric_compiler::CompilerCondition::kCompilerInvariant,
            tempo_tracing::LogSeverity::kError,
            "tuple arity is too large");
    auto genericTupleType = lyric_common::TypeDef::forConcrete(
        genericTupleUrl, std::vector<lyric_common::TypeDef>(
            genericUnwrapTypeArguments.begin(), genericUnwrapTypeArguments.end()));
    auto genericGenericReceiverType = lyric_common::TypeDef::forConcrete(
        fundamentalUnwrap, {genericUnwrapType, genericTupleType});

    return block->resolveImpl(concreteReceiverType,
        { genericConcreteReceiverType, genericGenericReceiverType});
}

tempo_utils::Status
lyric_compiler::UnpackHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto *symbolCache = driver->getSymbolCache();
    auto *typeSystem = driver->getTypeSystem();

    // if there are no unwrap params, then we are done
    if (m_unpack.tupleTypeArguments.empty())
        return {};

    // resolve the impl implementing unwrap() for the specified unwrap type and tuple type
    lyric_assembler::ImplReference implRef;
    TU_ASSIGN_OR_RETURN (implRef, resolve_unwrap_impl(
        m_unpack.unwrapType, m_unpack.tupleTypeArguments, block, driver));

    lyric_assembler::CallableInvoker extensionInvoker;
    TU_RETURN_IF_NOT_OK (prepare_impl_action(kUnwrapExtensionName, implRef, extensionInvoker, block, symbolCache));

    auto &usingRef = implRef.usingRef;
    auto instanceTypeArguments = usingRef.typeDef.getConcreteArguments();
    std::vector<lyric_common::TypeDef> callsiteTypeArguments(
        instanceTypeArguments.begin(), instanceTypeArguments.end());
    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(extensionInvoker, callsiteTypeArguments));

    TU_RETURN_IF_NOT_OK (m_fragment->loadRef(m_unpack.targetRef));
    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(m_unpack.unwrapType));

    // invoke unwrap
    lyric_common::TypeDef tupleType;
    TU_ASSIGN_OR_RETURN (tupleType, extensionInvoker.invoke(block, reifier, m_fragment));
    auto tupleUrl = tupleType.getConcreteUrl();

    // declare a temporary to store the result of the unwrap
    lyric_assembler::DataReference tupleRef;
    TU_ASSIGN_OR_RETURN (tupleRef, block->declareTemporary(tupleType, /* isVariable= */ false));

    // store the tuple in the temporary
    TU_RETURN_IF_NOT_OK (m_fragment->storeRef(tupleRef, /* initialStore= */ true));

    // store each tuple element in a local variable
    for (tu_uint32 i = 0; i < m_unpack.tupleTypeArguments.size(); i++) {
        auto tupleMember = absl::StrCat("t", i);

        lyric_assembler::AbstractSymbol *receiver;
        TU_ASSIGN_OR_RETURN (receiver, symbolCache->getOrImportSymbol(tupleUrl));

        lyric_assembler::DataReference memberRef;
        switch (receiver->getSymbolType()) {
            case lyric_assembler::SymbolType::CLASS: {
                auto *classSymbol = cast_symbol_to_class(receiver);
                lyric_typing::MemberReifier memberReifier(typeSystem, tupleType, classSymbol->classTemplate());
                TU_ASSIGN_OR_RETURN (memberRef, classSymbol->resolveMember(tupleMember, memberReifier, tupleType));
                break;
            }
            default:
                return block->logAndContinue(CompilerCondition::kInvalidSymbol,
                    tempo_tracing::LogSeverity::kError,
                    "invalid receiver symbol {}", tupleUrl.toString());
        }

        // load tuple onto the top of the stack
        TU_RETURN_IF_NOT_OK (m_fragment->loadRef(tupleRef));

        // load member from tuple onto the top of the stack
        TU_RETURN_IF_NOT_OK (m_fragment->loadRef(memberRef));

        // drop the previous result from the stack
        auto &unwrapRef = m_unpack.unwrapRefs[i];
        TU_RETURN_IF_NOT_OK (m_fragment->storeRef(unwrapRef.second, /* initialStore= */ true));
    }

    // pop the tuple off the stack
    TU_RETURN_IF_NOT_OK (m_fragment->popValue());

    return {};
}

lyric_compiler::UnwrapParam::UnwrapParam(
    Unpack *unpack,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_unpack(unpack),
      m_fragment(fragment)
{
    TU_ASSERT (m_unpack != nullptr);
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::UnwrapParam::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();

    if (!node->isClass(lyric_schema::kLyricAstParamClass))
        return block->logAndContinue(CompilerCondition::kCompilerInvariant,
            tempo_tracing::LogSeverity::kError,
            "expected Unwrap node");

    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec paramSpec;
    TU_ASSIGN_OR_RETURN (paramSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));
    lyric_common::TypeDef paramType;
    TU_ASSIGN_OR_RETURN (paramType, typeSystem->resolveAssignable(block, paramSpec));
    m_unpack->tupleTypeArguments.push_back(paramType);

    std::string paramName;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, paramName));
    lyric_assembler::DataReference ref;
    TU_ASSIGN_OR_RETURN (ref, block->declareVariable(
        paramName, lyric_object::AccessType::Private, paramType, /* isVariable= */ true));
    m_unpack->unwrapRefs.emplace_back(paramName, ref);

    return {};
}