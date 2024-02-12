#include <tempo_utils/log_stream.h>

#include <lyric_assembler/code_builder.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_compiler/internal/compile_conditional.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::internal::compile_cond(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();
    auto *state = moduleEntry.getState();
    auto *fundamentalCache = state->fundamentalCache();

    // cond expression must have at least one case
    moduleEntry.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstCondClass, 1);

    lyric_assembler::CodeBuilder *code = block->blockCode();

    std::vector<lyric_assembler::CondCasePatch> patchList;
    lyric_common::TypeDef returnType;  // initially invalid, will be set by first case

    // evaluate each case
    for (int i = 0; i < walker.numChildren(); i++) {
        auto condCase = walker.getChild(i);
        if (condCase.numChildren() != 2)
            block->throwSyntaxError(condCase, "invalid cond case");

        auto makeLabelResult = code->makeLabel();
        if (makeLabelResult.isStatus())
            return makeLabelResult.getStatus();
        auto predicateLabel = makeLabelResult.getResult();

        // evaluate the case predicate
        auto predicateResult = compile_expression(block, condCase.getChild(0), moduleEntry);
        if (predicateResult.isStatus())
            return predicateResult;
        auto predicateType = predicateResult.getResult();

        auto boolType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);
        if (!typeSystem->isAssignable(boolType, predicateType))
            return state->logAndContinue(CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "case predicate must return Boolean");

        auto predicateJumpResult = code->jumpIfFalse();
        if (predicateJumpResult.isStatus())
            return predicateJumpResult.getStatus();
        auto predicateJump = predicateJumpResult.getResult();

        lyric_assembler::BlockHandle consequent(block->blockProc(), block->blockCode(), block, block->blockState());

        // evaluate the case consequent
        auto consequentResult = compile_expression(&consequent, condCase.getChild(1), moduleEntry);
        if (consequentResult.isStatus())
            return consequentResult;
        auto consequentType = consequentResult.getResult();

        auto consequentJumpResult = code->jump();
        if (consequentJumpResult.isStatus())
            return consequentJumpResult.getStatus();
        auto consequentJump = consequentJumpResult.getResult();

        // update the return type
        if (returnType.isValid()) {
            auto unifyResult = typeSystem->unifyAssignable(consequentType, returnType);
            if (unifyResult.isStatus())
                return unifyResult.getStatus();
            returnType = unifyResult.getResult();
        } else {
            returnType = consequentType;
        }

        patchList.push_back(lyric_assembler::CondCasePatch(predicateLabel, predicateJump, consequentJump));
    }

    tempo_utils::Status status;

    // construct the alternative block
    auto alternativeBlockEnter = code->makeLabel();
    if (alternativeBlockEnter.isStatus())
        return alternativeBlockEnter.getStatus();

    lyric_assembler::BlockHandle alternative(block->blockProc(), block->blockCode(), block, block->blockState());

    // evaluate the alternative block. if no alternative is specified, then by default we return Nil
    lyric_common::TypeDef alternativeType;
    if (walker.hasAttr(lyric_parser::kLyricAstDefaultOffset)) {
        tu_uint32 defaultOffset;
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstDefaultOffset, defaultOffset);
        auto alternativeResult = compile_expression(&alternative, walker.getNodeAtOffset(defaultOffset), moduleEntry);
        if (alternativeResult.isStatus())
            return alternativeResult.getStatus();
        alternativeType = alternativeResult.getResult();
    } else {
        status = code->loadNil();
        if (!status.isOk())
            return status;
        // it is intentional that we don't set alternativeType here
    }

    // determine the final return type
    if (alternativeType.isValid()) {
        // if alternative branch was defined then unify its type with the other case branches
        auto unifyResult = typeSystem->unifyAssignable(alternativeType, returnType);
        if (unifyResult.isStatus())
            return unifyResult.getStatus();
        returnType = unifyResult.getResult();
    } else {
        // otherwise the return type is the union of return type and Nil
        auto nilType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Nil);
        returnType = lyric_common::TypeDef::forUnion({returnType, nilType});
    }

    auto alternativeBlockExit = code->makeLabel();
    if (alternativeBlockExit.isStatus())
        return alternativeBlockExit.getStatus();

    // patch jumps for each case expression except the last
    tu_uint32 i = 0;
    for (; i < patchList.size() - 1; i++) {
        auto &casePatch = patchList[i];
        auto &nextCasePatch = patchList[i + 1];
        status = code->patch(casePatch.getPredicateJump(), nextCasePatch.getPredicateLabel());
        if (!status.isOk())
            return status;

        status = code->patch(casePatch.getConsequentJump(), alternativeBlockExit.getResult());
        if (!status.isOk())
            return status;
    }

    // patch jumps for the last case expression
    auto &lastCasePatch = patchList[i];
    status = code->patch(lastCasePatch.getPredicateJump(), alternativeBlockEnter.getResult());
    if (!status.isOk())
        return status;

    status = code->patch(lastCasePatch.getConsequentJump(), alternativeBlockExit.getResult());
    if (!status.isOk())
        return status;

    return returnType;
}

tempo_utils::Status
lyric_compiler::internal::compile_if(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();
    auto *state = moduleEntry.getState();

    // cond expression must have at least one case
    if (walker.numChildren() == 0)
        block->throwSyntaxError(walker, "empty if");

    lyric_assembler::CodeBuilder *code = block->blockCode();

    std::vector<lyric_assembler::CondCasePatch> patchList;

    // evaluate each case
    for (int i = 0; i < walker.numChildren(); i++) {
        auto condCase = walker.getChild(i);
        if (condCase.numChildren() != 2)
            block->throwSyntaxError(condCase, "invalid if case");

        auto makeLabelResult = code->makeLabel();
        if (makeLabelResult.isStatus())
            return makeLabelResult.getStatus();
        auto predicateLabel = makeLabelResult.getResult();

        // evaluate the case predicate
        auto predicateResult = compile_expression(block, condCase.getChild(0), moduleEntry);
        if (predicateResult.isStatus())
            return predicateResult.getStatus();
        auto predicateType = predicateResult.getResult();

        auto boolType = block->blockState()->fundamentalCache()->getFundamentalType(
            lyric_assembler::FundamentalSymbol::Bool);
        if (!typeSystem->isAssignable(boolType, predicateType))
            return state->logAndContinue(CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "case predicate must return Boolean");

        auto predicateJumpResult = code->jumpIfFalse();
        if (predicateJumpResult.isStatus())
            return predicateJumpResult.getStatus();
        auto predicateJump = predicateJumpResult.getResult();

        lyric_assembler::BlockHandle consequent(block->blockProc(), block->blockCode(), block, block->blockState());

        // evaluate the case consequent
        auto consequentResult = compile_block(&consequent, condCase.getChild(1), moduleEntry);
        if (consequentResult.isStatus())
            return consequentResult.getStatus();
        auto consequentType = consequentResult.getResult();

        // discard intermediate expression result
        if (consequentType.isValid())
            code->popValue();

        auto consequentJumpResult = code->jump();
        if (consequentJumpResult.isStatus())
            return consequentJumpResult.getStatus();
        auto consequentJump = consequentJumpResult.getResult();

        patchList.push_back(lyric_assembler::CondCasePatch(predicateLabel, predicateJump, consequentJump));
    }

    tempo_utils::Status status;

    // construct the alternative block
    auto alternativeBlockEnter = code->makeLabel();
    if (alternativeBlockEnter.isStatus())
        return alternativeBlockEnter.getStatus();

    lyric_assembler::BlockHandle alternative(block->blockProc(), block->blockCode(), block, block->blockState());

    // evaluate the alternative block. if no alternative is specified, then by default we return Nil
    if (walker.hasAttr(lyric_parser::kLyricAstDefaultOffset)) {
        tu_uint32 defaultOffset;
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstDefaultOffset, defaultOffset);
        auto alternativeResult = compile_block(&alternative, walker.getNodeAtOffset(defaultOffset), moduleEntry);
        if (alternativeResult.isStatus())
            return alternativeResult.getStatus();
        auto alternativeType = alternativeResult.getResult();

        // discard intermediate expression result
        if (alternativeType.isValid())
            code->popValue();
    }

    auto alternativeBlockExit = code->makeLabel();
    if (alternativeBlockExit.isStatus())
        return alternativeBlockExit.getStatus();

    // patch jumps for each case expression except the last
    tu_uint32 i = 0;
    for (; i < patchList.size() - 1; i++) {
        auto &casePatch = patchList[i];
        auto &nextCasePatch = patchList[i + 1];
        status = code->patch(casePatch.getPredicateJump(), nextCasePatch.getPredicateLabel());
        if (!status.isOk())
            return status;

        status = code->patch(casePatch.getConsequentJump(), alternativeBlockExit.getResult());
        if (!status.isOk())
            return status;
    }

    // patch jumps for the last case expression
    auto &lastCasePatch = patchList[i];
    status = code->patch(lastCasePatch.getPredicateJump(), alternativeBlockEnter.getResult());
    if (!status.isOk())
        return status;

    status = code->patch(lastCasePatch.getConsequentJump(), alternativeBlockExit.getResult());
    if (!status.isOk())
        return status;

    return CompilerStatus::ok();
}
