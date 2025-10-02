
#include <lyric_compiler/compiler_utils.h>
#include <lyric_compiler/def_handler.h>
#include <lyric_compiler/pack_handler.h>
#include <lyric_compiler/proc_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::DefHandler::DefHandler(
    bool isSideEffect,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect),
      m_currentNamespace(nullptr)
{
}

lyric_compiler::DefHandler::DefHandler(
    bool isSideEffect,
    lyric_assembler::NamespaceSymbol *currentNamespace,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect),
      m_currentNamespace(currentNamespace)
{
    TU_ASSERT (m_currentNamespace != nullptr);
}

tempo_utils::Status
lyric_compiler::DefHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_VV << "before DefHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();

    // get function name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // get function access level
    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    lyric_typing::TemplateSpec templateSpec;
    lyric_assembler::ParameterPack parameterPack;
    lyric_typing::TypeSpec returnSpec;

    // if function is generic, then parse the template parameter list
    if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        lyric_parser::ArchetypeNode *genericNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
        TU_ASSIGN_OR_RETURN (templateSpec, typeSystem->parseTemplate(block, genericNode->getArchetypeNode()));
    }

    // declare the function call
    TU_ASSIGN_OR_RETURN (m_function.callSymbol, block->declareFunction(identifier,
        isHidden, templateSpec.templateParameters));

    // add function to the current namespace if specified
    if (m_currentNamespace != nullptr) {
        TU_RETURN_IF_NOT_OK (m_currentNamespace->putTarget(m_function.callSymbol->getSymbolUrl()));
    }

    auto *resolver = m_function.callSymbol->callResolver();
    auto *packNode = node->getChild(0);

    // parse the parameter pack
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(block, packNode->getArchetypeNode()));

    // resolve the parameter pack
    TU_ASSIGN_OR_RETURN (parameterPack, typeSystem->resolvePack(resolver, packSpec));

    // parse the return type
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    TU_ASSIGN_OR_RETURN (returnSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));

    // resolve the return type
    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, typeSystem->resolveAssignable(resolver, returnSpec));

    //
    TU_ASSIGN_OR_RETURN (m_function.procHandle,
        m_function.callSymbol->defineCall(parameterPack, returnType));

    auto pack = std::make_unique<PackHandler>(m_function.callSymbol, block, driver);
    ctx.appendGrouping(std::move(pack));

    auto proc = std::make_unique<DefProc>(&m_function, block, driver);
    ctx.appendChoice(std::move(proc));

    return {};
}

tempo_utils::Status
lyric_compiler::DefHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_VV << "after DefHandler@" << this;

    auto *driver = getDriver();
    auto *fragment = m_function.procHandle->procFragment();

    // add return instruction
    TU_RETURN_IF_NOT_OK (fragment->returnToCaller());

    // finalize the call
    TU_RETURN_IF_STATUS (m_function.callSymbol->finalizeCall());

    if (!m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}

lyric_compiler::DefProc::DefProc(
    Function *function,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_function(function)
{
    TU_ASSERT (m_function != nullptr);
}

tempo_utils::Status
lyric_compiler::DefProc::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto handler = std::make_unique<ProcHandler>(
        m_function->procHandle, /* requiresResult= */ true, getBlock(), getDriver());
    ctx.setGrouping(std::move(handler));
    return {};
}