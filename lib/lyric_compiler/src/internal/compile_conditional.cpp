#include <tempo_utils/log_stream.h>

#include <lyric_assembler/fundamental_cache.h>
#include <lyric_compiler/internal/compile_block.h>
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

    // cond expression must have at least one branch
    moduleEntry.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstCondClass, 1);

    auto *blockCode = block->blockCode();
    auto *fragment = blockCode->rootFragment();

    std::vector<lyric_assembler::CondWhenPatch> patchList;
    lyric_common::TypeDef returnType;  // initially invalid, will be set by first case

    // evaluate each branch
    for (int i = 0; i < walker.numChildren(); i++) {
        auto condWhen = walker.getChild(i);
        moduleEntry.checkClassAndChildCountOrThrow(condWhen, lyric_schema::kLyricAstWhenClass, 2);

        lyric_assembler::JumpLabel predicateLabel;
        TU_ASSIGN_OR_RETURN (predicateLabel, fragment->appendLabel());

        // evaluate the predicate
        auto predicateResult = compile_expression(block, condWhen.getChild(0), moduleEntry);
        if (predicateResult.isStatus())
            return predicateResult;
        auto predicateType = predicateResult.getResult();

        auto boolType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);

        bool isAssignable;
        TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(boolType, predicateType));
        if (!isAssignable)
            return state->logAndContinue(CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "cond predicate must return Bool");

        lyric_assembler::JumpTarget predicateJump;
        TU_ASSIGN_OR_RETURN (predicateJump, fragment->jumpIfFalse());

        lyric_assembler::BlockHandle consequent(block->blockProc(), block->blockCode(), block, block->blockState());

        // evaluate the consequent
        auto consequentResult = compile_expression(&consequent, condWhen.getChild(1), moduleEntry);
        if (consequentResult.isStatus())
            return consequentResult;
        auto consequentType = consequentResult.getResult();

        lyric_assembler::JumpTarget consequentJump;
        TU_ASSIGN_OR_RETURN (consequentJump, fragment->unconditionalJump());

        // update the return type
        if (returnType.isValid()) {
            auto unifyResult = typeSystem->unifyAssignable(consequentType, returnType);
            if (unifyResult.isStatus())
                return unifyResult.getStatus();
            returnType = unifyResult.getResult();
        } else {
            returnType = consequentType;
        }

        patchList.emplace_back(predicateLabel, predicateJump, consequentJump);
    }

    tempo_utils::Status status;

    // construct the alternative block
    lyric_assembler::JumpLabel alternativeBlockEnter;
    TU_ASSIGN_OR_RETURN (alternativeBlockEnter, fragment->appendLabel());

    lyric_assembler::BlockHandle alternative(block->blockProc(), block->blockCode(), block, block->blockState());

    // evaluate the alternative block. if no alternative is specified, then by default we return Nil
    lyric_common::TypeDef alternativeType;
    if (walker.hasAttr(lyric_parser::kLyricAstDefaultOffset)) {
        lyric_parser::NodeWalker defaultNode;
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstDefaultOffset, defaultNode);
        auto alternativeResult = compile_expression(&alternative, defaultNode, moduleEntry);
        if (alternativeResult.isStatus())
            return alternativeResult.getStatus();
        alternativeType = alternativeResult.getResult();
    } else {
        TU_RETURN_IF_NOT_OK (fragment->immediateNil());
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

    lyric_assembler::JumpLabel alternativeBlockExit;
    TU_ASSIGN_OR_RETURN (alternativeBlockExit, fragment->appendLabel());

    // patch jumps for each expression except the last
    tu_uint32 i = 0;
    for (; i < patchList.size() - 1; i++) {
        auto &condWhenPatch = patchList[i];
        auto &nextPatch = patchList[i + 1];
        TU_RETURN_IF_NOT_OK (fragment->patchTarget(condWhenPatch.getPredicateJump(), nextPatch.getPredicateLabel()));
        TU_RETURN_IF_NOT_OK (fragment->patchTarget(condWhenPatch.getConsequentJump(), alternativeBlockExit));
    }

    // patch jumps for the last expression
    auto &lastPatch = patchList[i];
    TU_RETURN_IF_NOT_OK (fragment->patchTarget(lastPatch.getPredicateJump(), alternativeBlockEnter));
    TU_RETURN_IF_NOT_OK (fragment->patchTarget(lastPatch.getConsequentJump(), alternativeBlockExit));

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
    moduleEntry.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstIfClass, 1);

    auto *blockCode = block->blockCode();
    auto *fragment = blockCode->rootFragment();

    std::vector<lyric_assembler::CondWhenPatch> patchList;

    // evaluate each case
    for (int i = 0; i < walker.numChildren(); i++) {
        auto condWhen = walker.getChild(i);
        moduleEntry.checkClassAndChildCountOrThrow(condWhen, lyric_schema::kLyricAstWhenClass, 2);

        lyric_assembler::JumpLabel predicateLabel;
        TU_ASSIGN_OR_RETURN (predicateLabel, fragment->appendLabel());

        // evaluate the case predicate
        auto predicateResult = compile_expression(block, condWhen.getChild(0), moduleEntry);
        if (predicateResult.isStatus())
            return predicateResult.getStatus();
        auto predicateType = predicateResult.getResult();

        auto boolType = block->blockState()->fundamentalCache()->getFundamentalType(
            lyric_assembler::FundamentalSymbol::Bool);

        bool isAssignable;
        TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(boolType, predicateType));
        if (!isAssignable)
            return state->logAndContinue(CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "case predicate must return Boolean");

        lyric_assembler::JumpTarget predicateJump;
        TU_ASSIGN_OR_RETURN (predicateJump, fragment->jumpIfFalse());

        lyric_assembler::BlockHandle consequent(block->blockProc(), block->blockCode(), block, block->blockState());

        // evaluate the case consequent
        auto consequentResult = compile_block(&consequent, condWhen.getChild(1), moduleEntry);
        if (consequentResult.isStatus())
            return consequentResult.getStatus();
        auto consequentType = consequentResult.getResult();

        // discard intermediate expression result
        if (consequentType.isValid()) {
            TU_RETURN_IF_NOT_OK (fragment->popValue());
        }

        lyric_assembler::JumpTarget consequentJump;
        TU_ASSIGN_OR_RETURN (consequentJump, fragment->unconditionalJump());

        patchList.push_back(lyric_assembler::CondWhenPatch(predicateLabel, predicateJump, consequentJump));
    }

    tempo_utils::Status status;

    // construct the alternative block
    lyric_assembler::JumpLabel alternativeBlockEnter;
    TU_ASSIGN_OR_RETURN (alternativeBlockEnter, fragment->appendLabel());

    lyric_assembler::BlockHandle alternative(block->blockProc(), block->blockCode(), block, block->blockState());

    // evaluate the alternative block. if no alternative is specified, then by default we return Nil
    if (walker.hasAttr(lyric_parser::kLyricAstDefaultOffset)) {
        lyric_parser::NodeWalker defaultNode;
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstDefaultOffset, defaultNode);
        auto alternativeResult = compile_block(&alternative, defaultNode, moduleEntry);
        if (alternativeResult.isStatus())
            return alternativeResult.getStatus();
        auto alternativeType = alternativeResult.getResult();

        // discard intermediate expression result
        if (alternativeType.isValid()) {
            TU_RETURN_IF_NOT_OK (fragment->popValue());
        }
    }

    lyric_assembler::JumpLabel alternativeBlockExit;
    TU_ASSIGN_OR_RETURN (alternativeBlockExit, fragment->appendLabel());

    // patch jumps for each expression except the last
    tu_uint32 i = 0;
    for (; i < patchList.size() - 1; i++) {
        auto &casePatch = patchList[i];
        auto &nextCasePatch = patchList[i + 1];
        TU_RETURN_IF_NOT_OK (fragment->patchTarget(casePatch.getPredicateJump(), nextCasePatch.getPredicateLabel()));
        TU_RETURN_IF_NOT_OK (fragment->patchTarget(casePatch.getConsequentJump(), alternativeBlockExit));
    }

    // patch jumps for the last expression
    auto &lastPatch = patchList[i];
    TU_RETURN_IF_NOT_OK (fragment->patchTarget(lastPatch.getPredicateJump(), alternativeBlockEnter));
    TU_RETURN_IF_NOT_OK (fragment->patchTarget(lastPatch.getConsequentJump(), alternativeBlockExit));

    return CompilerStatus::ok();
}
