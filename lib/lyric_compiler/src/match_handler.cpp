
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_set.h>
#include <lyric_compiler/block_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/match_handler.h>
#include <lyric_compiler/match_utils.h>
#include <lyric_compiler/unpack_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::MatchHandler::MatchHandler(
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
lyric_compiler::MatchHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    auto numChildren = node->numChildren();
    if (numChildren == 0)
        return block->logAndContinue(CompilerCondition::kCompilerInvariant,
            tempo_tracing::LogSeverity::kError,
            "missing target for match expression");

    auto expression = std::make_unique<FormChoice>(
        FormType::Expression, m_fragment, block, driver);
    ctx.appendChoice(std::move(expression));
    numChildren--;

    if (numChildren == 0)
        return block->logAndContinue(CompilerCondition::kCompilerInvariant,
            tempo_tracing::LogSeverity::kError,
            "empty match expression");

    auto consequent = std::make_unique<MatchConsequent>();
    consequent->block = std::make_unique<lyric_assembler::BlockHandle>(
        block->blockProc(), block, block->blockState());

    auto initial = std::make_unique<MatchInitial>(
        &m_match, consequent.get(), m_fragment, /* requiresResult= */ true, block, driver);
    m_match.consequents.push_back(std::move(consequent));
    ctx.appendChoice(std::move(initial));
    numChildren--;

    for (int i = 0; i < numChildren; i++) {
        consequent = std::make_unique<MatchConsequent>();
        consequent->block = std::make_unique<lyric_assembler::BlockHandle>(
            block->blockProc(), block, block->blockState());
        auto when = std::make_unique<MatchWhen>(
            &m_match, consequent.get(), m_fragment, /* requiresResult= */ true, block, driver);
        m_match.consequents.push_back(std::move(consequent));
        ctx.appendGrouping(std::move(when));
    }

    if (node->hasAttr(lyric_parser::kLyricAstDefaultOffset)) {
        m_match.alternative = std::make_unique<MatchAlternative>();
        auto dfl = std::make_unique<MatchDefault>(
            m_match.alternative.get(), m_fragment, /* requiresResult= */ true, block, driver);
        ctx.appendChoice(std::move(dfl));
    }

    return {};
}

tempo_utils::Status
lyric_compiler::MatchHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    // if there was a default then pop the expression type
    if (m_match.alternative) {
        m_match.alternative->alternativeType = driver->peekResult();
        TU_RETURN_IF_NOT_OK (driver->popResult());
    } else {
        TU_RETURN_IF_NOT_OK (m_fragment->immediateNil());
        //m_match.alternative->alternativeType = fundamentalCache->getFundamentalType(
        //    lyric_assembler::FundamentalSymbol::Nil);
    }

    lyric_assembler::JumpLabel exitLabel;
    TU_ASSIGN_OR_RETURN (exitLabel, m_fragment->appendLabel());

    auto &consequents = m_match.consequents;
    auto &alternative = m_match.alternative;

    // patch jumps for each expression except the last
    for (int i = 0; i < consequents.size() - 1; i++) {
        auto &matchWhenPatch = consequents[i]->patch;
        auto &nextPatch = consequents[i + 1]->patch;
        TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(matchWhenPatch.getPredicateJump(), nextPatch.getPredicateLabel()));
        TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(matchWhenPatch.getConsequentJump(), exitLabel));
    }

    // patch jumps for the last expression
    auto &lastPatch = consequents.back()->patch;
    if (alternative != nullptr) {
        TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(lastPatch.getPredicateJump(), alternative->enterLabel));
    } else {
        TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(lastPatch.getPredicateJump(), exitLabel));
    }
    TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(lastPatch.getConsequentJump(), exitLabel));

    lyric_assembler::UnifiedTypeSet resultSet(block->blockState());

    // unify the set of consequent types
    for (int i = 1; i < consequents.size(); i++) {
        auto &consequentType = consequents[i]->consequentType;
        TU_RETURN_IF_NOT_OK (resultSet.putType(consequentType));
    }

    // if alternative exists then add the result to the type set. if there is no alternative clause
    // then determine whether the match is exhaustive by verifying that all subtypes of the target are
    // enumerable and there is a branch for all subtypes.
    if (alternative != nullptr) {
        auto &alternativeType = alternative->alternativeType;
        TU_RETURN_IF_NOT_OK (resultSet.putType(alternativeType));
    } else {
        //auto NilType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Nil);
        //TU_RETURN_IF_NOT_OK (resultSet.putType(NilType));
        TU_RETURN_IF_NOT_OK (check_match_is_exhaustive(&m_match, block, driver));
    }

    auto unifiedType = resultSet.getUnifiedType();

    // if cond is a side effect then pop the value, otherwise push the result
    if (m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (m_fragment->popValue());
    } else {
        TU_RETURN_IF_NOT_OK (driver->pushResult(unifiedType));
    }

    return {};
}

lyric_compiler::MatchInitial::MatchInitial(
    Match *match,
    MatchConsequent *consequent,
    lyric_assembler::CodeFragment *fragment,
    bool isExpression,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_match(match),
      m_consequent(consequent),
      m_fragment(fragment),
      m_isExpression(isExpression)
{
    TU_ASSERT (m_match != nullptr);
    TU_ASSERT (m_consequent != nullptr);
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::MatchInitial::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto targetType = driver->peekResult();
    TU_ASSERT (driver->popResult());

    // create a temporary local variable for the target
    TU_ASSIGN_OR_RETURN (m_match->targetRef, block->declareTemporary(targetType, /* isVariable= */ false));

    // store the result of the target in the temporary
    TU_RETURN_IF_NOT_OK (m_fragment->storeRef(m_match->targetRef, /* initialStore= */ true));

    auto when = std::make_unique<MatchWhen>(
        m_match, m_consequent, m_fragment, m_isExpression, block, driver);
    ctx.setGrouping(std::move(when));

    return {};
}

lyric_compiler::MatchWhen::MatchWhen(
    Match *match,
    MatchConsequent *consequent,
    lyric_assembler::CodeFragment *fragment,
    bool isExpression,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_match(match),
      m_consequent(consequent),
      m_fragment(fragment),
      m_isExpression(isExpression)
{
    TU_ASSERT (m_consequent != nullptr);
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::MatchWhen::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    TU_ASSIGN_OR_RETURN (m_consequent->jumpLabel, m_fragment->appendLabel());

    auto predicate = std::make_unique<MatchPredicate>(
        m_match, m_consequent, m_fragment, block, driver);
    ctx.appendChoice(std::move(predicate));

    auto body = std::make_unique<MatchBody>(
        m_consequent, m_fragment, m_isExpression, m_consequent->block.get(), driver);
    ctx.appendChoice(std::move(body));

    return {};
}

tempo_utils::Status
lyric_compiler::MatchWhen::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *driver = getDriver();

    // pop the expression type
    m_consequent->consequentType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    // if consequent is a statement and returns a result then pop the value
    if (!m_isExpression) {
        if (m_consequent->consequentType.getType() != lyric_common::TypeDefType::NoReturn) {
            TU_RETURN_IF_NOT_OK (m_fragment->popValue());
        }
    }

    lyric_assembler::JumpTarget consequentJump;
    TU_ASSIGN_OR_RETURN (consequentJump, m_fragment->unconditionalJump());

    m_consequent->patch = lyric_assembler::MatchWhenPatch(m_consequent->predicateType,
        m_consequent->jumpLabel, m_consequent->jumpTarget, consequentJump, m_consequent->consequentType);

    return {};
}

lyric_compiler::MatchPredicate::MatchPredicate(
    Match *match,
    MatchConsequent *consequent,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_match(match),
      m_consequent(consequent),
      m_fragment(fragment)
{
    TU_ASSERT (m_consequent != nullptr);
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::MatchPredicate::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    TU_LOG_INFO << "decide MatchPredicate@" << this << ": "
                << resource->getNsUrl() << "#" << resource->getName();

    auto *block = getBlock();
    auto *driver = getDriver();

    switch (astId) {
        case lyric_schema::LyricAstId::Unpack: {
            auto unpack = std::make_unique<MatchUnpackPredicate>(
                m_match, m_consequent, m_fragment, block, driver);
            ctx.setChoice(std::move(unpack));
            return {};
        }
        case lyric_schema::LyricAstId::SymbolRef: {
            auto symbolref = std::make_unique<MatchSymbolRefPredicate>(
                m_match, m_consequent, m_fragment, block, driver);
            ctx.setChoice(std::move(symbolref));
            return {};
        }
        default:
            return block->logAndContinue(CompilerCondition::kSyntaxError,
                tempo_tracing::LogSeverity::kError,
                "invalid match predicate");
    }
}

lyric_compiler::MatchUnpackPredicate::MatchUnpackPredicate(
    Match *match,
    MatchConsequent *consequent,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_match(match),
      m_consequent(consequent),
      m_fragment(fragment)
{
    TU_ASSERT (m_consequent != nullptr);
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::MatchUnpackPredicate::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();

    // resolve the predicate type
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec predicateSpec;
    TU_ASSIGN_OR_RETURN (predicateSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));
    lyric_common::TypeDef predicateType;
    TU_ASSIGN_OR_RETURN (predicateType, typeSystem->resolveAssignable(block, predicateSpec));

    // check whether the predicate matches the target
    TU_ASSIGN_OR_RETURN (m_consequent->predicateType, compile_predicate(
        m_match->targetRef, predicateType, m_fragment, block, driver));

    // if the type comparison is <= 0, then invoke the consequent
    TU_ASSIGN_OR_RETURN (m_consequent->jumpTarget, m_fragment->jumpIfGreaterThan());

    auto unpack = std::make_unique<UnpackHandler>(
        m_consequent->predicateType, m_match->targetRef, m_fragment, m_consequent->block.get(), driver);
    ctx.setGrouping(std::move(unpack));

    return {};
}

lyric_compiler::MatchSymbolRefPredicate::MatchSymbolRefPredicate(
    Match *match,
    MatchConsequent *consequent,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_match(match),
      m_consequent(consequent),
      m_fragment(fragment)
{
    TU_ASSERT (m_consequent != nullptr);
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::MatchSymbolRefPredicate::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto *symbolCache = driver->getSymbolCache();

    // get the predicate symbol from the symbol ref
    lyric_common::SymbolPath symbolPath;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath));
    if (!symbolPath.isValid())
        return block->logAndContinue(lyric_compiler::CompilerCondition::kInvalidSymbol,
            tempo_tracing::LogSeverity::kError,
            "invalid symbol path");

    // resolve the match when symbol
    lyric_common::SymbolUrl symbolUrl;
    TU_ASSIGN_OR_RETURN (symbolUrl, block->resolveDefinition(symbolPath));
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(symbolUrl));

    // determine the predicate type
    lyric_common::TypeDef predicateType;
    switch (symbol->getSymbolType()) {
        case lyric_assembler::SymbolType::INSTANCE:
            predicateType = cast_symbol_to_instance(symbol)->getTypeDef();
            break;
        case lyric_assembler::SymbolType::ENUM:
            predicateType = cast_symbol_to_enum(symbol)->getTypeDef();
            break;
        default:
            return block->logAndContinue(lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "invalid match {}; predicate must be an instance or an enum",
                symbol->getSymbolUrl().toString());
    }

    // check whether the predicate matches the target
    TU_ASSIGN_OR_RETURN (m_consequent->predicateType, compile_predicate(
        m_match->targetRef, predicateType, m_fragment, block, driver));

    // if the type comparison is <= 0, then invoke the consequent
    TU_ASSIGN_OR_RETURN (m_consequent->jumpTarget, m_fragment->jumpIfGreaterThan());

    return {};
}

lyric_compiler::MatchBody::MatchBody(
    MatchConsequent *consequent,
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
lyric_compiler::MatchBody::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    FormType formType = m_isExpression? FormType::Expression : FormType::Any;
    auto body = std::make_unique<FormChoice>(formType, m_fragment, block, driver);
    ctx.setChoice(std::move(body));
    return {};
}

lyric_compiler::MatchDefault::MatchDefault(
    MatchAlternative *alternative,
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
lyric_compiler::MatchDefault::decide(
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
