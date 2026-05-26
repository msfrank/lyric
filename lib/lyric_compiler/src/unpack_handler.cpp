
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/impl_utils.h>
#include <lyric_compiler/unpack_handler.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_typing/callsite_reifier.h>
#include <lyric_typing/member_reifier.h>

#include "lyric_typing/impl_selector.h"
#include "lyric_typing/summon_reifier.h"

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

static tempo_utils::Result<lyric_common::TypeDef>
invoke_unwrap(
    const lyric_assembler::DataReference &wrappedRef,
    const std::vector<lyric_common::TypeDef> &tupleTypeArguments,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment)
{
    auto *state = block->blockState();
    auto *fundamentalCache = state->fundamentalCache();
    auto *symbolCache = state->symbolCache();

    auto wrappedType = wrappedRef.typeDef;

    int arity = tupleTypeArguments.size();
    auto UnwrapUrl = fundamentalCache->getUnwrapUrl(arity);

    std::vector<lyric_common::TypeDef> unwrapArguments;
    unwrapArguments.push_back(wrappedType);
    unwrapArguments.insert(unwrapArguments.end(), tupleTypeArguments.cbegin(), tupleTypeArguments.cend());

    lyric_common::TypeDef unwrapType;
    TU_ASSIGN_OR_RETURN (unwrapType, lyric_common::TypeDef::forConcrete(UnwrapUrl, unwrapArguments));

    lyric_assembler::ConceptSymbol *unwrapConcept;
    TU_ASSIGN_OR_RETURN (unwrapConcept, symbolCache->getOrImportConcept(UnwrapUrl));
    auto *unwrapAction = unwrapConcept->getAction("Unwrap");

    // load wrapped reference onto the top of the stack as the first argument to unwrap
    TU_RETURN_IF_NOT_OK (fragment->loadRef(wrappedRef));

    lyric_typing::SummonReifier summoner(state);

    TU_RETURN_IF_NOT_OK (summoner.initialize(unwrapAction));
    TU_RETURN_IF_NOT_OK (summoner.reifyNextArgument(wrappedType));
    TU_RETURN_IF_NOT_OK (summoner.finalize());

    lyric_typing::ImplSelector selector(&summoner, block);
    std::unique_ptr<lyric_assembler::AbstractCallable> callable;
    TU_RETURN_IF_NOT_OK (selector.select(callable));

    lyric_typing::CallsiteReifier reifier(state);
    TU_RETURN_IF_NOT_OK (reifier.initialize(callable.get(), selector.getCallsiteArguments()));
    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(wrappedType));

    lyric_common::TypeDef resultType;
    TU_ASSIGN_OR_RETURN (resultType, callable->invoke(block, reifier, fragment));

    return reifier.reifyResult(resultType);
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

    // invoke unwrap for the specified wrapped type
    lyric_common::TypeDef tupleType;
    TU_ASSIGN_OR_RETURN (tupleType, invoke_unwrap(
        m_unpack.targetRef, m_unpack.tupleTypeArguments, block, m_fragment));

    // resolve the class symbol for the tuple
    lyric_assembler::ClassSymbol *classSymbol;
    TU_ASSIGN_OR_RETURN (classSymbol, symbolCache->getOrImportClass(tupleType.getConcreteUrl()));

    // declare a temporary to store the result of the unwrap
    lyric_assembler::DataReference tupleRef;
    TU_ASSIGN_OR_RETURN (tupleRef, block->declareTemporary(tupleType, /* isVariable= */ false));

    // store the tuple in the temporary
    TU_RETURN_IF_NOT_OK (m_fragment->storeRef(tupleRef, /* initialStore= */ true));

    // store each tuple element in a local variable
    for (tu_uint32 i = 0; i < m_unpack.tupleTypeArguments.size(); i++) {
        auto tupleMember = absl::StrCat("Element", i);

        lyric_assembler::DataReference memberRef;
        lyric_typing::MemberReifier memberReifier(typeSystem);
        TU_RETURN_IF_NOT_OK (memberReifier.initialize(tupleType, classSymbol->classTemplate()));
        TU_ASSIGN_OR_RETURN (memberRef, classSymbol->resolveMember(tupleMember, memberReifier, tupleType));

        // load tuple onto the top of the stack
        TU_RETURN_IF_NOT_OK (m_fragment->loadRef(tupleRef));

        // load member from tuple onto the top of the stack
        TU_RETURN_IF_NOT_OK (m_fragment->loadRef(memberRef));

        // drop the previous result from the stack
        auto &unwrapRef = m_unpack.unwrapRefs[i];
        TU_RETURN_IF_NOT_OK (m_fragment->storeRef(unwrapRef.second, /* initialStore= */ true));
    }

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
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
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
        paramName, /* isHidden= */ true, paramType, /* isVariable= */ true));
    m_unpack->unwrapRefs.emplace_back(paramName, ref);

    return {};
}