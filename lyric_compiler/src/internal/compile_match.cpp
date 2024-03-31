
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/disjoint_type_set.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/internal/compile_match.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_compiler/internal/compile_operator.h>
#include <lyric_compiler/internal/compile_unwrap.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/unify_assignable.h>
#include <tempo_utils/log_stream.h>

static tempo_utils::Status
compile_predicate(
    lyric_assembler::BlockHandle *block,
    const lyric_assembler::SymbolBinding &target,
    const lyric_common::TypeDef &predicateType,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (predicateType.isValid());

    auto *state = block->blockState();
    auto *code = block->blockCode();
    tempo_utils::Status status;

    // push target variable onto the top of the stack
    status = block->load(target);
    if (!status.isOk())
        return status;

    // pop target and push target type descriptor onto the stack
    status = code->writeOpcode(lyric_object::Opcode::OP_TYPE_OF);
    if (!status.isOk())
        return status;

    if (!state->typeCache()->hasType(predicateType))
        block->throwAssemblerInvariant("missing predicate type {}", predicateType.toString());
    state->typeCache()->touchType(predicateType);

    // push isA type descriptor onto the stack
    switch (predicateType.getType()) {
        case lyric_common::TypeDefType::Concrete: {
            auto *typeHandle = state->typeCache()->getType(predicateType);
            status = code->loadType(typeHandle->getAddress());
            if (!status.isOk())
                return status;
            break;
        }
        default:
            block->throwAssemblerInvariant("invalid predicate type {}", predicateType.toString());
    }

    // verify that the targetType can be a subtype of isAType
    TU_RETURN_IF_NOT_OK (
        lyric_compiler::internal::match_types(target.type, predicateType, walker, block, moduleEntry));

    // perform type comparison
    return code->writeOpcode(lyric_object::Opcode::OP_TYPE_CMP);
}

static tempo_utils::Result<lyric_assembler::MatchCasePatch>
compile_match_case_symbol_ref(
    lyric_assembler::BlockHandle *block,
    const lyric_assembler::SymbolBinding &target,
    const lyric_parser::NodeWalker &symbolRef,
    const lyric_parser::NodeWalker &body,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (symbolRef.isValid());
    TU_ASSERT (body.isValid());

    auto *code = block->blockCode();
    auto *state = block->blockState();

    // get the match case symbol from the symbol ref
    lyric_common::SymbolPath symbolPath;
    moduleEntry.parseAttrOrThrow(symbolRef, lyric_parser::kLyricAstSymbolPath, symbolPath);
    if (!symbolPath.isValid())
        block->throwSyntaxError(symbolRef, "invalid symbol path");

    // resolve the match case symbol
    auto resolveMatchCaseSymbol = block->resolveDefinition(symbolPath);
    if (resolveMatchCaseSymbol.isStatus())
        return resolveMatchCaseSymbol.getStatus();
    auto matchCaseSymbolUrl = resolveMatchCaseSymbol.getResult();
    if (!state->symbolCache()->hasSymbol(matchCaseSymbolUrl))
        return block->logAndContinue(lyric_compiler::CompilerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "missing symbol {}", matchCaseSymbolUrl.toString());
    auto *sym = state->symbolCache()->getSymbol(matchCaseSymbolUrl);

    // determine the predicate type
    lyric_common::TypeDef predicateType;
    switch (sym->getSymbolType()) {
        case lyric_assembler::SymbolType::INSTANCE:
            predicateType = cast_symbol_to_instance(sym)->getAssignableType();
            break;
        case lyric_assembler::SymbolType::ENUM:
            predicateType = cast_symbol_to_enum(sym)->getAssignableType();
            break;
        default:
            return block->logAndContinue(lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "invalid match case {}; predicate must be an instance or an enum", sym->getSymbolUrl().toString());
    }

    // body is the consequent block
    moduleEntry.checkClassOrThrow(body, lyric_schema::kLyricAstBlockClass);

    auto makeLabelResult = code->makeLabel();
    if (makeLabelResult.isStatus())
        return makeLabelResult.getStatus();
    auto predicateLabel = makeLabelResult.getResult();

    // check whether the predicate case matches the target
    auto status = compile_predicate(block, target, predicateType, symbolRef, moduleEntry);
    if (!status.isOk())
        return status;

    // if the type comparison is <= 0, then invoke the consequent
    auto predicateJumpResult = code->jumpIfGreaterThan();
    if (predicateJumpResult.isStatus())
        return predicateJumpResult.getStatus();
    auto predicateJump = predicateJumpResult.getResult();

    // evaluate the consequent block
    lyric_assembler::BlockHandle consequent(block->blockProc(), block->blockCode(), block, block->blockState());
    auto consequentResult = lyric_compiler::internal::compile_block(&consequent, body, moduleEntry);
    if (consequentResult.isStatus())
        return consequentResult.getStatus();
    auto consequentType = consequentResult.getResult();

    auto consequentJumpResult = code->jump();
    if (consequentJumpResult.isStatus())
        return consequentJumpResult.getStatus();
    auto consequentJump = consequentJumpResult.getResult();

    return lyric_assembler::MatchCasePatch(predicateType, predicateLabel,
        predicateJump, consequentJump, consequentType);
}

static tempo_utils::Result<lyric_assembler::MatchCasePatch>
compile_match_case_unpack(
    lyric_assembler::BlockHandle *block,
    const lyric_assembler::SymbolBinding &target,
    const lyric_parser::NodeWalker &unpack,
    const lyric_parser::NodeWalker &body,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (unpack.isValid());
    TU_ASSERT (body.isValid());

    auto *code = block->blockCode();

    // get the assignable type of the unpack
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(unpack, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto assignedType = unpack.getNodeAtOffset(typeOffset);
    auto *typeSystem = moduleEntry.getTypeSystem();
    auto resolveUnwrapType = typeSystem->resolveAssignable(block, assignedType);
    if (resolveUnwrapType.isStatus())
        return resolveUnwrapType.getStatus();
    auto predicateType = resolveUnwrapType.getResult();

    // body is the consequent block
    moduleEntry.checkClassOrThrow(body, lyric_schema::kLyricAstBlockClass);

    auto makeLabelResult = code->makeLabel();
    if (makeLabelResult.isStatus())
        return makeLabelResult.getStatus();
    auto predicateLabel = makeLabelResult.getResult();

    // check whether the predicate case matches the target
    auto status = compile_predicate(block, target, predicateType, unpack, moduleEntry);
    if (!status.isOk())
        return status;

    // if the type comparison is <= 0, then invoke the consequent
    auto predicateJumpResult = code->jumpIfGreaterThan();
    if (predicateJumpResult.isStatus())
        return predicateJumpResult.getStatus();
    auto predicateJump = predicateJumpResult.getResult();

    // create a block scope for the consequent and declare any unwrap variables
    lyric_assembler::BlockHandle consequent(block->blockProc(), block->blockCode(), block, block->blockState());
    status = lyric_compiler::internal::compile_unwrap(&consequent, unpack, predicateType, target, moduleEntry);
    if (!status.isOk())
        return status;

    // evaluate the consequent block
    auto consequentResult = lyric_compiler::internal::compile_block(&consequent, body, moduleEntry);
    if (consequentResult.isStatus())
        return consequentResult.getStatus();
    auto consequentType = consequentResult.getResult();

    auto consequentJumpResult = code->jump();
    if (consequentJumpResult.isStatus())
        return consequentJumpResult.getStatus();
    auto consequentJump = consequentJumpResult.getResult();

    return lyric_assembler::MatchCasePatch(predicateType, predicateLabel,
        predicateJump, consequentJump, consequentType);
}

static tempo_utils::Result<lyric_assembler::MatchCasePatch>
compile_match_case(
    lyric_assembler::BlockHandle *block,
    const lyric_assembler::SymbolBinding &target,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    moduleEntry.checkClassAndChildCountOrThrow(walker, lyric_schema::kLyricAstCaseClass, 2);
    auto casePredicate = walker.getChild(0);
    auto caseBody = walker.getChild(1);

    lyric_schema::LyricAstId predicateId{};
    moduleEntry.parseIdOrThrow(casePredicate, lyric_schema::kLyricAstVocabulary, predicateId);

    // first child of case is the unwrap matcher
    switch (predicateId) {
        case lyric_schema::LyricAstId::Unpack:
            return compile_match_case_unpack(block, target, casePredicate, caseBody, moduleEntry);
        case lyric_schema::LyricAstId::SymbolRef:
            return compile_match_case_symbol_ref(block, target, casePredicate, caseBody, moduleEntry);
        default:
            block->throwSyntaxError(casePredicate, "invalid match case predicate");
    }
}

/**
 *
 * @param targetType
 * @param patchList
 * @return
 */
static tempo_utils::Result<bool>
check_concrete_target_is_exhaustive(
    const lyric_common::TypeDef &targetType,
    const std::vector<lyric_assembler::MatchCasePatch> &patchList,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    auto *state = moduleEntry.getState();

    auto *sym = state->symbolCache()->getSymbol(targetType.getConcreteUrl());
    if (sym == nullptr)
        return block->logAndContinue(lyric_compiler::CompilerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "missing symbol {}", targetType.getConcreteUrl().toString());

    lyric_object::DeriveType derive;
    absl::flat_hash_set<lyric_common::TypeDef> targetSet;

    switch (sym->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(sym);
            derive = classSymbol->getDeriveType();
            if (derive == lyric_object::DeriveType::Sealed) {
                targetSet.insert(classSymbol->sealedTypesBegin(), classSymbol->sealedTypesEnd());
            }
            break;
        }
        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = cast_symbol_to_enum(sym);
            derive = enumSymbol->getDeriveType();
            if (derive == lyric_object::DeriveType::Sealed) {
                targetSet.insert(enumSymbol->sealedTypesBegin(), enumSymbol->sealedTypesEnd());
            }
            break;
        }
        case lyric_assembler::SymbolType::EXISTENTIAL: {
            auto *existentialSymbol = cast_symbol_to_existential(sym);
            derive = existentialSymbol->getDeriveType();
            if (derive == lyric_object::DeriveType::Sealed) {
                targetSet.insert(existentialSymbol->sealedTypesBegin(), existentialSymbol->sealedTypesEnd());
            }
            break;
        }
        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = cast_symbol_to_instance(sym);
            derive = instanceSymbol->getDeriveType();
            if (derive == lyric_object::DeriveType::Sealed) {
                targetSet.insert(instanceSymbol->sealedTypesBegin(), instanceSymbol->sealedTypesEnd());
            }
            break;
        }
        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(sym);
            derive = structSymbol->getDeriveType();
            if (derive == lyric_object::DeriveType::Sealed) {
                targetSet.insert(structSymbol->sealedTypesBegin(), structSymbol->sealedTypesEnd());
            }
            break;
        }
        default:
            return block->logAndContinue(lyric_compiler::CompilerCondition::kSyntaxError,
                tempo_tracing::LogSeverity::kError,
                "type {} cannot be used as the target of a match expression",
                targetType.getConcreteUrl().toString());
    }

    if (derive == lyric_object::DeriveType::Any) {
        // if target symbol is not sealed or final then we can't exhaustively check
        return patchList.size() == 1 && patchList.front().getPredicateType() == targetType;
    }

    // if target symbol is final then target set is empty so insert the target type as the only member
    if (derive == lyric_object::DeriveType::Final) {
        targetSet.insert(targetType);
    }

    // verify each member in the target set matches a case
    for (const auto &matchCase : patchList) {
        auto predicateType = matchCase.getPredicateType();
        auto iterator = targetSet.find(predicateType);
        if (iterator != targetSet.cend()) {
            targetSet.erase(iterator);
        } else {
            block->logAndContinue(lyric_compiler::CompilerCondition::kSyntaxError,
                tempo_tracing::LogSeverity::kWarn,
                "predicate type {} is not a subtype of target type {}",
                predicateType.toString(), targetType.getConcreteUrl().toString());
        }
    }

    // if target set is empty then the matching is exhaustive
    return targetSet.empty();
}

/**
 *
 * @param targetType
 * @param patchList
 * @return
 */
static tempo_utils::Result<bool>
check_union_target_is_exhaustive(
    const lyric_common::TypeDef &targetType,
    const std::vector<lyric_assembler::MatchCasePatch> &patchList,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    for (const auto &memberType : targetType.getUnionMembers()) {
        bool isExhaustive;
        TU_ASSIGN_OR_RETURN (isExhaustive, check_concrete_target_is_exhaustive(
            memberType, patchList, block, moduleEntry));
        if (!isExhaustive)
            return false;
    }

    // if every union member matches then matching is exhaustive
    return true;
}

/**
 *
 * @param block
 * @param walker
 * @param moduleEntry
 * @return
 */
tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::internal::compile_match(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());
    auto *fundamentalCache = block->blockState()->fundamentalCache();

    moduleEntry.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstMatchClass, 2);

    auto *code = block->blockCode();
    tempo_utils::Status status;

    // evaluate the match target and push the result onto the top of the stack
    auto compileTargetResult = compile_expression(block, walker.getChild(0), moduleEntry);
    if (compileTargetResult.isStatus())
        return compileTargetResult;
    auto targetType = compileTargetResult.getResult();

    // create a temporary local variable for the target
    auto declareTempResult = block->declareTemporary(targetType, lyric_parser::BindingType::VALUE);
    if (declareTempResult.isStatus())
        return declareTempResult.getStatus();
    auto target = declareTempResult.getResult();

    // store the result of the target in the temporary
    status = block->store(target);
    if (!status.isOk())
        return status;

    std::vector<lyric_assembler::MatchCasePatch> patchList;
    lyric_assembler::DisjointTypeSet predicateSet(block->blockState());

    // evaluate each case
    for (int i = 1; i < walker.numChildren(); i++) {
        auto matchCase = walker.getChild(i);

        auto matchCaseResult = compile_match_case(block, target, matchCase, moduleEntry);
        if (matchCaseResult.isStatus())
            return matchCaseResult.getStatus();
        auto casePatch = matchCaseResult.getResult();

        // verify all case predicate types are disjoint
        TU_RETURN_IF_NOT_OK (predicateSet.putType(casePatch.getPredicateType()));

        patchList.push_back(casePatch);
    }

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
        auto defaultCase = walker.getNodeAtOffset(defaultOffset);
        auto alternativeResult = compile_block(&alternative, defaultCase, moduleEntry);
        if (alternativeResult.isStatus())
            return alternativeResult;
        alternativeType = alternativeResult.getResult();
        TU_RETURN_IF_NOT_OK (predicateSet.putType(alternativeType));
    } else {
        status = code->loadNil();
        if (!status.isOk())
            return status;
        TU_RETURN_IF_NOT_OK (predicateSet.putType(
            fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Nil)));
        // it is intentional that we don't set alternativeType here
    }

    auto alternativeBlockExit = code->makeLabel();
    if (alternativeBlockExit.isStatus())
        return alternativeBlockExit.getStatus();

    // patch jumps for each case expression except the last
    tu_uint32 j = 0;
    for (; j < patchList.size() - 1; j++) {
        const auto &casePatch = patchList.at(j);
        const auto &nextCasePatch = patchList.at(j + 1);
        status = code->patch(casePatch.getPredicateJump(), nextCasePatch.getPredicateLabel());
        if (!status.isOk())
            return status;

        status = code->patch(casePatch.getConsequentJump(), alternativeBlockExit.getResult());
        if (!status.isOk())
            return status;
    }

    // patch jumps for the last case expression
    const auto &lastCasePatch = patchList.at(j);
    status = code->patch(lastCasePatch.getPredicateJump(), alternativeBlockEnter.getResult());
    if (!status.isOk())
        return status;

    status = code->patch(lastCasePatch.getConsequentJump(), alternativeBlockExit.getResult());
    if (!status.isOk())
        return status;

    // unify the types of all case clauses
    auto unifiedType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    for (const auto &casePatch : patchList) {
        auto unifyResult = lyric_typing::unify_assignable(
            casePatch.getConsequentType(), unifiedType, block->blockState());
        if (unifyResult.isStatus())
            return unifyResult.getStatus();
        unifiedType = unifyResult.getResult();
    }

    // if alternative clause is defined then unify its type with the other case clauses and return
    if (alternativeType.isValid())
        return lyric_typing::unify_assignable(alternativeType, unifiedType, block->blockState());

    // if there is no alternative clause, then determine whether the match is exhaustive by verifying
    // that all subtypes of the target are enumerable and there is a case for all subtypes.
    bool isExhaustive = false;
    switch (targetType.getType()) {
        case lyric_common::TypeDefType::Concrete: {
            TU_ASSIGN_OR_RETURN (isExhaustive, check_concrete_target_is_exhaustive(
                targetType, patchList, block, moduleEntry));
            break;
        }
        case lyric_common::TypeDefType::Union: {
            TU_ASSIGN_OR_RETURN (isExhaustive, check_union_target_is_exhaustive(
                targetType, patchList, block, moduleEntry));
            break;
        }
        default:
            //block->logAndContinue(CompilerCondition::kSyntaxError,
            //    tempo_tracing::LogSeverity::kError,
            //    "match expression has no default case and target type {} cannot be checked exhaustively",
            //    targetType.toString());
            break;
    }

    if (isExhaustive)
        return unifiedType;

    // otherwise the return type is the union of return type and Nil
    auto nilType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Nil);
    return lyric_common::TypeDef::forUnion({unifiedType, nilType});
}
