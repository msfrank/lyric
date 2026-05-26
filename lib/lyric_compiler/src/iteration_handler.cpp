
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/block_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/iteration_handler.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_typing/callsite_reifier.h>
#include <lyric_typing/impl_selector.h>
#include <lyric_typing/summon_reifier.h>

lyric_compiler::WhileHandler::WhileHandler(
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
lyric_compiler::WhileHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *proc = block->blockProc();
    auto *driver = getDriver();

    lyric_assembler::JumpLabel topOfLoop;
    TU_ASSIGN_OR_RETURN (topOfLoop, m_fragment->appendLabel());
    TU_ASSIGN_OR_RETURN (m_loop.loopHandle, proc->declareLoop(topOfLoop));

    auto predicate = std::make_unique<FormChoice>(
        FormType::Expression, m_fragment, block, driver);
    ctx.appendChoice(std::move(predicate));

    auto whileBody = std::make_unique<WhileBody>(&m_loop, m_fragment, block, driver);
    ctx.appendChoice(std::move(whileBody));

    return {};
}

tempo_utils::Status
lyric_compiler::WhileHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *driver = getDriver();
    auto *loopHandle = m_loop.loopHandle;

    lyric_assembler::JumpTarget nextIteration;
    TU_ASSIGN_OR_RETURN (nextIteration, m_fragment->unconditionalJump());
    TU_RETURN_IF_NOT_OK (loopHandle->continueLoop(nextIteration));

    lyric_assembler::JumpLabel loopExit;
    TU_ASSIGN_OR_RETURN (loopExit, m_fragment->appendLabel());
    TU_RETURN_IF_NOT_OK (loopHandle->finalizeLoop(loopExit));

    // if handler is not a side effect then push a NoReturn result
    if (!m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}

lyric_compiler::WhileBody::WhileBody(
    Loop *loop,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_loop(loop),
      m_fragment(fragment)
{
    TU_ASSERT (m_loop != nullptr);
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::WhileBody::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto *fundamentalCache = driver->getFundamentalCache();
    auto *typeSystem = driver->getTypeSystem();

    auto BoolType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);

    auto testType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    bool isAssignable;
    TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(BoolType, testType));
    if (!isAssignable)
        return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
            "expected test expression to return {}; found {}", BoolType.toString(), testType.toString());

    // break loop implicitly if predicate returns false
    lyric_assembler::JumpTarget predicateJump;
    TU_ASSIGN_OR_RETURN (predicateJump, m_fragment->jumpIfFalse());
    TU_RETURN_IF_NOT_OK (m_loop->loopHandle->breakLoop(predicateJump));

    auto whileBlock = std::make_unique<lyric_assembler::BlockHandle>(
        block->blockProc(), block, block->blockState());
    auto whileBody = std::make_unique<BlockHandler>(
        std::move(whileBlock), /* requiresResult= */ false, /* isSideEffect= */ true,
        m_fragment, driver);
    ctx.setGrouping(std::move(whileBody));

    return {};
}

lyric_compiler::ForHandler::ForHandler(
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
lyric_compiler::ForHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();

    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, m_iteration.targetIdentifier));

    // resolve the target type if it was specified
    lyric_common::TypeDef targetType;
    if (node->hasAttr(lyric_parser::kLyricAstTypeOffset)) {
        lyric_parser::ArchetypeNode *typeNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
        lyric_typing::TypeSpec targetSpec;
        TU_ASSIGN_OR_RETURN (targetSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));
        TU_ASSIGN_OR_RETURN (m_iteration.targetType, typeSystem->resolveAssignable(block, targetSpec));
    }

    // handle the generator expression
    auto generator = std::make_unique<FormChoice>(
        FormType::Expression, m_fragment, block, driver);
    ctx.appendChoice(std::move(generator));

    // handle the for body
    auto body = std::make_unique<ForBody>(&m_iteration, m_fragment, block, driver);
    ctx.appendChoice(std::move(body));

    return {};
}

tempo_utils::Status
lyric_compiler::ForHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *driver = getDriver();
    auto *loopHandle = m_iteration.loopHandle;

    lyric_assembler::JumpTarget nextIteration;
    TU_ASSIGN_OR_RETURN (nextIteration, m_fragment->unconditionalJump());
    TU_RETURN_IF_NOT_OK (loopHandle->continueLoop(nextIteration));

    lyric_assembler::JumpLabel loopExit;
    TU_ASSIGN_OR_RETURN (loopExit, m_fragment->appendLabel());
    TU_RETURN_IF_NOT_OK (loopHandle->finalizeLoop(loopExit));

    // if handler is not a side effect then push a NoReturn result
    if (!m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}

lyric_compiler::ForBody::ForBody(
    Iteration *iteration,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_iteration(iteration),
      m_fragment(fragment)
{
    TU_ASSERT (m_iteration != nullptr);
    TU_ASSERT (m_fragment != nullptr);
}

/**
 * if generator base type is not Iterator then we need to construct an iterator
 */
static tempo_utils::Result<lyric_common::TypeDef>
summon_iterator(
    lyric_compiler::Iteration *iteration,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem,
    lyric_assembler::CodeFragment *fragment)
{
    TU_NOTNULL (iteration);
    auto *state = block->blockState();
    auto *fundamentalCache = state->fundamentalCache();
    auto *symbolCache = state->symbolCache();

    auto IterableUrl = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Iterable);
    lyric_assembler::ConceptSymbol *iterableConcept;
    TU_ASSIGN_OR_RETURN (iterableConcept, symbolCache->getOrImportConcept(IterableUrl));

    auto *iterateAction = iterableConcept->getAction("Iterate");

    lyric_common::TypeDef iterableType;
    TU_ASSIGN_OR_RETURN (iterableType, lyric_common::TypeDef::forConcrete(IterableUrl, {iteration->generatorType}));

    lyric_typing::SummonReifier summoner(state);

    TU_RETURN_IF_NOT_OK (summoner.initialize(iterateAction));
    TU_RETURN_IF_NOT_OK (summoner.reifyNextArgument(iteration->generatorType));
    TU_RETURN_IF_NOT_OK (summoner.finalize());

    lyric_typing::ImplSelector selector(&summoner, block);
    std::unique_ptr<lyric_assembler::AbstractCallable> callable;
    TU_RETURN_IF_NOT_OK (selector.select(callable));

    lyric_typing::CallsiteReifier reifier(state);
    TU_RETURN_IF_NOT_OK (reifier.initialize(callable.get(), selector.getCallsiteArguments()));
    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(iteration->generatorType));

    lyric_common::TypeDef resultType;
    TU_ASSIGN_OR_RETURN (resultType, callable->invoke(block, reifier, fragment));

    return reifier.reifyResult(resultType);
}

static bool
get_element_type_if_iterator(
    const lyric_common::TypeDef &generatorType,
    lyric_common::TypeDef &elementType,
    lyric_assembler::FundamentalCache *fundamentalCache)
{
    auto IteratorUrl = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Iterator);
    if (generatorType.getType() != lyric_common::TypeDefType::Concrete)
        return false;
    if (generatorType.getConcreteUrl() != IteratorUrl)
        return false;
    auto typeArguments = generatorType.getConcreteArguments();
    if (typeArguments.size() != 1)
        return false;
    elementType = typeArguments[0];
    return true;
}

tempo_utils::Status
lyric_compiler::ForBody::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *proc = block->blockProc();
    auto *driver = getDriver();
    auto *fundamentalCache = driver->getFundamentalCache();
    auto *symbolCache = driver->getSymbolCache();
    auto *typeSystem = driver->getTypeSystem();

    m_iteration->generatorType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    // look up the Iterator concept symbol
    auto IteratorUrl = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Iterator);
    lyric_assembler::ConceptSymbol *iteratorConcept;
    TU_ASSIGN_OR_RETURN (iteratorConcept, symbolCache->getOrImportConcept(IteratorUrl));

    lyric_common::TypeDef iteratorType;
    lyric_common::TypeDef elementType;

    // if the generator base type is not Iterator then summon the iterator
    iteratorType = m_iteration->generatorType;
    if (!get_element_type_if_iterator(m_iteration->generatorType, elementType, fundamentalCache)) {
        TU_ASSIGN_OR_RETURN (iteratorType, summon_iterator(m_iteration, block, typeSystem, m_fragment));
        if (!get_element_type_if_iterator(iteratorType, elementType, fundamentalCache))
            return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
                "could not summon iterator from {}", m_iteration->generatorType.toString());
    }

    // if target type is defined then verify element type is assignable
    if (m_iteration->targetType.isValid()) {
        bool isAssignable;
        TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(m_iteration->targetType, elementType));
        if (!isAssignable)
            return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
                "iterator element type {} cannot be assigned to target type {}",
                elementType.toString(), m_iteration->targetType.toString());
    } else {
        m_iteration->targetType = elementType;
    }

    // declare temp variable to store the iterator
    lyric_assembler::DataReference iteratorRef;
    TU_ASSIGN_OR_RETURN (iteratorRef, block->declareTemporary(iteratorType, /* isVariable= */ false));

    // store the iterator in the temp variable
    TU_RETURN_IF_NOT_OK (m_fragment->storeRef(iteratorRef, /* initialStore= */ true));

    // create a new block for the body of the loop
    m_iteration->forBlock = std::make_unique<lyric_assembler::BlockHandle>(proc, block, block->blockState());
    auto *forBlock = m_iteration->forBlock.get();

    // declare loop
    lyric_assembler::JumpLabel topOfLoop;
    TU_ASSIGN_OR_RETURN (topOfLoop, m_fragment->appendLabel());
    TU_ASSIGN_OR_RETURN (m_iteration->loopHandle, proc->declareLoop(topOfLoop));

    // declare the target variable which stores the value yielded from the iterator on each loop iteration
    lyric_assembler::DataReference targetRef;
    TU_ASSIGN_OR_RETURN (targetRef, forBlock->declareVariable(
        m_iteration->targetIdentifier, /* isHidden= */ true,
        m_iteration->targetType, /* isVariable= */ true));

    // push the iterator onto the stack, and invoke Valid() method
    TU_RETURN_IF_NOT_OK (m_fragment->loadRef(iteratorRef));

    // resolve Iterator.Valid() method
    std::unique_ptr<lyric_assembler::AbstractCallable> validCallable;
    TU_RETURN_IF_NOT_OK (iteratorConcept->prepareAction("Valid", iteratorType, validCallable));

    // invoke Valid method
    lyric_typing::CallsiteReifier validReifier(typeSystem);
    TU_RETURN_IF_NOT_OK (validReifier.initialize(validCallable.get(), {targetRef.typeDef}));

    lyric_common::TypeDef validReturnType;
    TU_ASSIGN_OR_RETURN (validReturnType, validCallable->invoke(forBlock, validReifier, m_fragment));

    auto boolType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);

    bool isAssignable;
    TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(boolType, validReturnType));
    if (!isAssignable)
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "expected Valid method to return {}; found {}",
            boolType.toString(), validReturnType.toString());

    // break loop implicitly if predicate returns false
    lyric_assembler::JumpTarget predicateJump;
    TU_ASSIGN_OR_RETURN (predicateJump, m_fragment->jumpIfFalse());
    TU_RETURN_IF_NOT_OK (m_iteration->loopHandle->breakLoop(predicateJump));

    // push the iterator onto the stack, and invoke Next() method
    TU_RETURN_IF_NOT_OK (m_fragment->loadRef(iteratorRef));

    // resolve Iterator.Next() method
    std::unique_ptr<lyric_assembler::AbstractCallable> nextCallable;
    TU_RETURN_IF_NOT_OK (iteratorConcept->prepareAction("Next", iteratorType, nextCallable));

    // invoke Next()
    lyric_typing::CallsiteReifier nextReifier(typeSystem);
    TU_RETURN_IF_NOT_OK (nextReifier.initialize(nextCallable.get(), {targetRef.typeDef}));
    lyric_common::TypeDef nextReturnType;
    TU_ASSIGN_OR_RETURN (nextReturnType, nextCallable->invoke(forBlock, nextReifier, m_fragment));

    TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(m_iteration->targetType, nextReturnType));
    if (!isAssignable)
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "expected Next method to return {}; found {}",
            m_iteration->targetType.toString(), nextReturnType.toString());

    // store the next value in the target variable
    TU_RETURN_IF_NOT_OK (m_fragment->storeRef(targetRef));

    auto group = std::make_unique<BlockHandler>(
        std::move(m_iteration->forBlock), /* requiresResult= */ false, /* isSideEffect= */ true,
        m_fragment, driver);
    ctx.setGrouping(std::move(group));

    return {};
}

lyric_compiler::ContinueHandler::ContinueHandler(
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_isSideEffect(isSideEffect),
      m_fragment(fragment)
{
    TU_NOTNULL (m_fragment);
}

tempo_utils::Status
lyric_compiler::ContinueHandler::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *proc = block->blockProc();
    auto *driver = getDriver();

    lyric_assembler::JumpTarget continueTarget;
    TU_ASSIGN_OR_RETURN (continueTarget, m_fragment->unconditionalJump());
    TU_RETURN_IF_NOT_OK (proc->continueCurrentLoop(continueTarget));

    // if handler is not a side effect then push a NoReturn result
    if (!m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}

lyric_compiler::BreakHandler::BreakHandler(
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_isSideEffect(isSideEffect),
      m_fragment(fragment)
{
    TU_NOTNULL (m_fragment);
}

tempo_utils::Status
lyric_compiler::BreakHandler::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *proc = block->blockProc();
    auto *driver = getDriver();

    lyric_assembler::JumpTarget breakTarget;
    TU_ASSIGN_OR_RETURN (breakTarget, m_fragment->unconditionalJump());
    TU_RETURN_IF_NOT_OK (proc->breakCurrentLoop(breakTarget));

    // if handler is not a side effect then push a NoReturn result
    if (!m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}
