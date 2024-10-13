
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_compiler/block_node_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/conditional_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::IfHandler::IfHandler(
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect),
      m_fragment(fragment)
{
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::IfHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    for (int i = 0; i < node->numChildren(); i++) {
        auto consequent = std::make_unique<WhenConsequent>();
        auto when = std::make_unique<WhenHandler>(
            consequent.get(), m_fragment, /* requiresResult= */ false, block, driver);
        m_conditional.consequents.push_back(std::move(consequent));
        ctx.appendGrouping(std::move(when));
    }

    if (node->hasAttr(lyric_parser::kLyricAstDefaultOffset)) {
        m_conditional.alternative = std::make_unique<WhenAlternative>();
        auto defaultBlock = std::make_unique<lyric_assembler::BlockHandle>(
            block->blockProc(), block, block->blockState());
        auto body = std::make_unique<BlockNodeHandler>(
            std::move(defaultBlock), /* requiresResult= */ false, /* isSideEffect= */ false, m_fragment, driver);
        ctx.appendGrouping(std::move(body));
    }

    return {};
}

tempo_utils::Status
lyric_compiler::IfHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *driver = getDriver();

    // if there was a default then pop the expression type
    if (m_conditional.alternative) {
        m_conditional.alternative->expressionType = driver->peekResult();
        TU_RETURN_IF_NOT_OK (driver->popResult());
        if (m_conditional.alternative->expressionType.getType() != lyric_common::TypeDefType::NoReturn) {
            TU_RETURN_IF_NOT_OK (m_fragment->popValue());
        }
    }

    lyric_assembler::JumpLabel exitLabel;
    TU_ASSIGN_OR_RETURN (exitLabel, m_fragment->appendLabel());

    auto &consequents = m_conditional.consequents;
    auto &alternative = m_conditional.alternative;

    // patch jumps for each consequent except the last
    for (int i = 0; i < consequents.size() - 1; i++) {
        auto &casePatch = consequents[i]->patch;
        auto &nextCasePatch = consequents[i + 1]->patch;
        TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(casePatch.getPredicateJump(), nextCasePatch.getPredicateLabel()));
        TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(casePatch.getConsequentJump(), exitLabel));
    }

    // patch jumps for the last consequent
    auto &lastPatch = consequents.back()->patch;
    if (alternative != nullptr) {
        TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(lastPatch.getPredicateJump(), alternative->enterLabel));
    } else {
        TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(lastPatch.getPredicateJump(), exitLabel));
    }
    TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(lastPatch.getConsequentJump(), exitLabel));

    // if handler is not a side effect then push a NoReturn result
    if (!m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}

lyric_compiler::CondHandler::CondHandler(
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect),
      m_fragment(fragment)
{
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::CondHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    for (int i = 0; i < node->numChildren(); i++) {
        auto consequent = std::make_unique<WhenConsequent>();
        auto when = std::make_unique<WhenHandler>(
            consequent.get(), m_fragment, /* requiresResult= */ true, block, driver);
        m_conditional.consequents.push_back(std::move(consequent));
        ctx.appendGrouping(std::move(when));
    }

    if (node->hasAttr(lyric_parser::kLyricAstDefaultOffset)) {
        m_conditional.alternative = std::make_unique<WhenAlternative>();
        auto dfl = std::make_unique<WhenDefault>(
            m_conditional.alternative.get(), m_fragment, /* requiresResult= */ true, block, driver);
        ctx.appendChoice(std::move(dfl));
    }

    return {};
}

tempo_utils::Status
lyric_compiler::CondHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *driver = getDriver();
    auto *fundamentalCache = driver->getFundamentalCache();
    auto *typeSystem = driver->getTypeSystem();

    // if there was a default then pop the expression type
    if (m_conditional.alternative) {
        m_conditional.alternative->expressionType = driver->peekResult();
        TU_RETURN_IF_NOT_OK (driver->popResult());
    } else {
        TU_RETURN_IF_NOT_OK (m_fragment->immediateNil());
        m_conditional.alternative->expressionType = fundamentalCache->getFundamentalType(
            lyric_assembler::FundamentalSymbol::Nil);
    }

    lyric_assembler::JumpLabel exitLabel;
    TU_ASSIGN_OR_RETURN (exitLabel, m_fragment->appendLabel());

    auto &consequents = m_conditional.consequents;
    auto &alternative = m_conditional.alternative;

    // patch jumps for each consequent except the last
    for (int i = 0; i < consequents.size() - 1; i++) {
        auto &casePatch = consequents[i]->patch;
        auto &nextCasePatch = consequents[i + 1]->patch;
        TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(casePatch.getPredicateJump(), nextCasePatch.getPredicateLabel()));
        TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(casePatch.getConsequentJump(), exitLabel));
    }

    // patch jumps for the last consequent
    auto &lastPatch = consequents.back()->patch;
    if (alternative != nullptr) {
        TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(lastPatch.getPredicateJump(), alternative->enterLabel));
    } else {
        TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(lastPatch.getPredicateJump(), exitLabel));
    }
    TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(lastPatch.getConsequentJump(), exitLabel));

    // unify the set of consequent types
    lyric_common::TypeDef resultType = consequents[0]->expressionType;
    for (int i = 1; i < consequents.size(); i++) {
        auto &consequentType = consequents[i]->expressionType;
        TU_ASSIGN_OR_RETURN (resultType, typeSystem->unifyAssignable(consequentType, resultType));
    }

    // if alternative exists then the result is a singular unified type, otherwise its
    if (alternative != nullptr) {
        auto &alternativeType = alternative->expressionType;
        TU_ASSIGN_OR_RETURN (resultType, typeSystem->unifyAssignable(alternativeType, resultType));
    } else {
        auto NilType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Nil);
        resultType = lyric_common::TypeDef::forUnion({resultType, NilType});
    }

    // if cond is a side effect then pop the value, otherwise push the result
    if (m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (m_fragment->popValue());
    } else {
        TU_RETURN_IF_NOT_OK (driver->pushResult(resultType));
    }

    return {};
}

lyric_compiler::WhenHandler::WhenHandler(
    WhenConsequent *consequent,
    lyric_assembler::CodeFragment *fragment,
    bool isExpression,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_consequent(consequent),
      m_fragment(fragment),
      m_isExpression(isExpression)
{
    TU_ASSERT (m_consequent != nullptr);
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::WhenHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    TU_ASSIGN_OR_RETURN (m_consequent->jumpLabel, m_fragment->appendLabel());

    auto predicate = std::make_unique<FormChoice>(
        FormType::Expression, m_fragment, block, driver);
    ctx.appendChoice(std::move(predicate));

    auto body = std::make_unique<WhenBody>(
        m_consequent, m_fragment, m_isExpression, block, driver);
    ctx.appendChoice(std::move(body));

    return {};
}

tempo_utils::Status
lyric_compiler::WhenHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto *fundamentalCache = driver->getFundamentalCache();
    auto *typeSystem = driver->getTypeSystem();

    // pop the expression type
    m_consequent->expressionType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    // pop the predicate type
    m_consequent->predicateType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    auto boolType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);

    bool isAssignable;
    TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(boolType, m_consequent->predicateType));
    if (!isAssignable)
        return block->logAndContinue(CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "case predicate must return Boolean");

    // if consequent is a statement and returns a result then pop the value
    if (!m_isExpression) {
        if (m_consequent->expressionType.getType() != lyric_common::TypeDefType::NoReturn) {
            TU_RETURN_IF_NOT_OK (m_fragment->popValue());
        }
    }

    lyric_assembler::JumpTarget consequentJump;
    TU_ASSIGN_OR_RETURN (consequentJump, m_fragment->unconditionalJump());

    m_consequent->patch = lyric_assembler::CondWhenPatch(
        m_consequent->jumpLabel, m_consequent->jumpTarget, consequentJump);

    return {};
}

lyric_compiler::WhenBody::WhenBody(
    WhenConsequent *consequent,
    lyric_assembler::CodeFragment *fragment,
    bool isExpression,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_consequent(consequent),
      m_fragment(fragment),
      m_isExpression(isExpression)
{
    TU_ASSERT (m_consequent != nullptr);
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::WhenBody::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    TU_ASSIGN_OR_RETURN (m_consequent->jumpTarget, m_fragment->jumpIfFalse());

    FormType formType = m_isExpression? FormType::Expression : FormType::Any;
    auto body = std::make_unique<FormChoice>(formType, m_fragment, block, driver);
    ctx.setChoice(std::move(body));
    return {};
}

lyric_compiler::WhenDefault::WhenDefault(
    WhenAlternative *alternative,
    lyric_assembler::CodeFragment *fragment,
    bool isExpression,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_alternative(alternative),
      m_fragment(fragment),
      m_isExpression(isExpression)
{
    TU_ASSERT (m_alternative != nullptr);
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::WhenDefault::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    TU_ASSIGN_OR_RETURN (m_alternative->enterLabel, m_fragment->appendLabel());

    FormType formType = m_isExpression? FormType::Expression : FormType::Any;
    auto body = std::make_unique<FormChoice>(formType, m_fragment, block, driver);
    ctx.setChoice(std::move(body));
    return {};
}
