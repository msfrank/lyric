
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/alias_utils.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/impl_handler.h>
#include <lyric_compiler/pack_handler.h>
#include <lyric_compiler/proc_handler.h>
#include <lyric_parser/ast_attrs.h>

#include "lyric_typing/overload_reifier.h"

lyric_compiler::ImplHandler::ImplHandler(
    Impl impl,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_impl(impl),
      m_reifier(driver->getState())
{
    TU_NOTNULL (m_impl.implHandle);
}

tempo_utils::Status
lyric_compiler::ImplHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_VV << "before ImplHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();

    if (!node->isClass(lyric_schema::kLyricAstImplClass))
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "expected Impl node");

    // reify the impl type
    TU_RETURN_IF_NOT_OK (m_reifier.initialize(m_impl.implType));

    for (auto it = node->childrenBegin(); it != node->childrenEnd(); it++) {
        auto *child = *it;

        lyric_schema::LyricAstId astId;
        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, astId));
        switch (astId) {
            case lyric_schema::LyricAstId::Alias: {
                lyric_assembler::BindingSymbol *bindingSymbol;
                TU_ASSIGN_OR_RETURN (bindingSymbol, declare_alias(child, block, typeSystem));
                TU_RETURN_IF_NOT_OK (m_reifier.reifyAliasArgument(bindingSymbol));
                break;
            }
            case lyric_schema::LyricAstId::Def:
                break;
            default:
                return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
                    "unexpected AST node");
        }
        auto definition = std::make_unique<ImplDefinition>(&m_impl, &m_reifier, block, driver);
        ctx.appendChoice(std::move(definition));
    }

    // after all aliases have been declared we define the contract
    lyric_common::TypeDef contractType;
    TU_ASSIGN_OR_RETURN (contractType, m_reifier.reifyContractType());
    TU_RETURN_IF_NOT_OK (m_impl.implHandle->defineContract(contractType));

    m_impl.contractArguments.insert(m_impl.contractArguments.cbegin(),
        contractType.concreteArgumentsBegin(), contractType.concreteArgumentsEnd());

    return {};
}

tempo_utils::Status
lyric_compiler::ImplHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    return {};
}

lyric_compiler::ImplDefinition::ImplDefinition(
    Impl *impl,
    lyric_typing::ImplReifier *reifier,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_impl(impl),
      m_reifier(reifier)
{
    TU_NOTNULL (m_impl);
    TU_NOTNULL (m_reifier);
}

tempo_utils::Status
lyric_compiler::ImplDefinition::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    lyric_schema::LyricAstId astId;
    TU_RETURN_IF_NOT_OK (node->parseId(lyric_schema::kLyricAstVocabulary, astId));
    switch (astId) {
        case lyric_schema::LyricAstId::Alias: {
            // alias was already defined so do nothing
            return {};
        }
        case lyric_schema::LyricAstId::Def: {
            auto handler = std::make_unique<ImplDef>(m_impl, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        default:
            return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
                "unexpected AST node");
    }
}

lyric_compiler::ImplDef::ImplDef(
    Impl *impl,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_impl(impl)
{
    TU_ASSERT (m_impl != nullptr);
}

tempo_utils::Status
lyric_compiler::ImplDef::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_VV << "before ImplDef@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();
    auto *implHandle = m_impl->implHandle;
    auto *implBlock = implHandle->implBlock();

    // get method name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // parse the return type
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec returnSpec;
    TU_ASSIGN_OR_RETURN (returnSpec, typeSystem->parseAssignable(implBlock, typeNode->getArchetypeNode()));

    // parse the parameter list
    auto *packNode = node->getChild(0);
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(implBlock, packNode->getArchetypeNode()));

    // resolve the parameter pack
    lyric_assembler::ParameterPack parameterPack;
    TU_ASSIGN_OR_RETURN (parameterPack, typeSystem->resolvePack(implBlock, packSpec));

    // resolve the return type
    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, typeSystem->resolveAssignable(implBlock, returnSpec));
    bool requiresResult = returnType != lyric_common::TypeDef::noReturn();

    auto *actionSymbol = implHandle->getAction(identifier);

    // reify the parameter pack and return type
    lyric_typing::OverloadReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(actionSymbol, m_impl->contractArguments));

    lyric_assembler::ParameterPack extensionParameters;
    TU_ASSIGN_OR_RETURN (extensionParameters, reifier.reifyParameters(parameterPack));
    lyric_common::TypeDef extensionResult;
    TU_ASSIGN_OR_RETURN (extensionResult, reifier.reifyResult(returnType));

    // verify that extension is compatible with action
    TU_RETURN_IF_NOT_OK (typeSystem->checkDispatchable(extensionParameters, extensionResult, parameterPack, returnType));

    // define the extension
    TU_ASSIGN_OR_RETURN (m_procHandle, implHandle->defineExtension(identifier, parameterPack, returnType));

    auto pack = std::make_unique<ExtensionPack>(block, driver);
    ctx.appendGrouping(std::move(pack));

    auto proc = std::make_unique<ProcHandler>(
        m_procHandle, requiresResult, getBlock(), getDriver());
    ctx.appendGrouping(std::move(proc));

    return {};
}

tempo_utils::Status
lyric_compiler::ImplDef::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_VV << "after ImplDef@" << this;

    auto *fragment = m_procHandle->procFragment();

    // add return instruction
    TU_RETURN_IF_NOT_OK (fragment->returnToCaller());

    // TODO: finalize the call

    return {};
}

lyric_compiler::ExtensionPack::ExtensionPack(
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver)
{
}

tempo_utils::Status
lyric_compiler::ExtensionPack::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    for (int i = 0; i < node->numChildren(); i++) {
        auto param = std::make_unique<ExtensionParam>(block, driver);
        ctx.appendChoice(std::move(param));
    }

    return {};
}

lyric_compiler::ExtensionParam::ExtensionParam(
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver)
{
}

tempo_utils::Status
lyric_compiler::ExtensionParam::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    if (!node->hasAttr(lyric_parser::kLyricAstDefaultOffset))
        return {};

    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    return CompilerStatus::forCondition(lyric_compiler::CompilerCondition::kSyntaxError,
        "parameter '{}' has unexpected initializer", identifier);
}