
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/impl_handler.h>
#include <lyric_compiler/pack_handler.h>
#include <lyric_compiler/proc_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::ImplHandler::ImplHandler(
    Impl impl,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_impl(impl)
{
    TU_ASSERT (m_impl.implHandle != nullptr);
}

tempo_utils::Status
lyric_compiler::ImplHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_VV << "before DefClassHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();

    if (!node->isClass(lyric_schema::kLyricAstImplClass))
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "expected Impl node");

    for (auto it = node->childrenBegin(); it != node->childrenEnd(); it++) {
        auto *child = *it;

        lyric_schema::LyricAstId astId;
        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, astId));
        switch (astId) {
            case lyric_schema::LyricAstId::Def: {
                auto def = std::make_unique<ImplDef>(&m_impl, block, driver);
                ctx.appendGrouping(std::move(def));
                break;
            }
            default:
                return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
                    "unexpected AST node");
        }
    }

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

    //
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

    auto *procBuilder = m_procHandle->procCode();
    auto *fragment = procBuilder->rootFragment();

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