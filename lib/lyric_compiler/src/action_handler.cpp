
#include <lyric_compiler/action_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/deref_utils.h>
#include <lyric_compiler/pack_handler.h>
#include <lyric_compiler/proc_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::ActionHandler::ActionHandler(
    Action action,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_action(action)
{
    TU_ASSERT (m_action.actionSymbol != nullptr);
}

tempo_utils::Status
lyric_compiler::ActionHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_VV << "before ActionHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();

    auto pack = std::make_unique<PackHandler>(block, driver);
    ctx.appendGrouping(std::move(pack));

    return {};
}

tempo_utils::Status
lyric_compiler::ActionHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::AfterContext &ctx)
{
    TU_LOG_VV << "after ActionHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();

    auto *resolver = m_action.actionSymbol->actionResolver();
    auto *packNode = node->getChild(0);

    // parse the parameter pack
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(block, packNode->getArchetypeNode()));

    // resolve the parameter pack
    lyric_assembler::ParameterPack parameterPack;
    TU_ASSIGN_OR_RETURN (parameterPack, typeSystem->resolvePack(resolver, packSpec));

    // parse the return type
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec returnSpec;
    TU_ASSIGN_OR_RETURN (returnSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));

    // resolve the return type
    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, typeSystem->resolveAssignable(resolver, returnSpec));

    // define the action
    TU_RETURN_IF_NOT_OK (m_action.actionSymbol->defineAction(parameterPack, returnType));

    return {};
}
