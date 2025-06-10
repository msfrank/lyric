
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/form_handler.h>
#include <lyric_compiler/using_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::UsingHandler::UsingHandler(
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect)
{
    m_using.fragment = fragment;
    TU_ASSERT (m_using.fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::UsingHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    if (!node->isClass(lyric_schema::kLyricAstUsingClass))
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "expected Using node");

    auto numChildren = node->numChildren();
    if (numChildren == 0)
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "Using node must have at least one child");
    numChildren--;

    auto usingRef = std::make_unique<UsingRef>(&m_using, block, driver);
    ctx.appendGrouping(std::move(usingRef));

    // if impl types were specified then handle them
    for (auto i = 0; i < numChildren; i++) {
        auto impl = std::make_unique<UsingImpl>(&m_using, block, driver);
        ctx.appendChoice(std::move(impl));
    }

    return {};
}

tempo_utils::Status
lyric_compiler::UsingHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *block = getBlock();
    TU_RETURN_IF_NOT_OK (block->useImpls(m_using.usingRef, m_using.implTypes));

    if (!m_isSideEffect) {
        auto *driver = getDriver();
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}

lyric_compiler::UsingRef::UsingRef(
    Using *using_,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_using(using_)
{
    TU_ASSERT (m_using != nullptr);
}

tempo_utils::Status
lyric_compiler::UsingRef::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto form = std::make_unique<FormChoice>(FormType::Expression, m_using->fragment, block, driver);
    ctx.appendChoice(std::move(form));
    return {};
}

tempo_utils::Status
lyric_compiler::UsingRef::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    // declare a temporary to store the using reference
    auto refType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());
    TU_ASSIGN_OR_RETURN (m_using->usingRef, block->declareTemporary(refType, /* isVariable= */ true));

    // store using reference in temporary
    TU_RETURN_IF_NOT_OK (m_using->fragment->storeRef(m_using->usingRef));

    return {};
}

lyric_compiler::UsingImpl::UsingImpl(
    Using *using_,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_using(using_)
{
    TU_ASSERT (m_using != nullptr);
}

tempo_utils::Status
lyric_compiler::UsingImpl::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();

    lyric_typing::TypeSpec implSpec;
    TU_ASSIGN_OR_RETURN (implSpec, typeSystem->parseAssignable(block, node->getArchetypeNode()));
    lyric_common::TypeDef implType;
    TU_ASSIGN_OR_RETURN (implType, typeSystem->resolveAssignable(block, implSpec));
    m_using->implTypes.insert(implType);

    return {};
}