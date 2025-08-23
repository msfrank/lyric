
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/block_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/iteration_handler.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_typing/callsite_reifier.h>

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
    auto *driver = getDriver();

    TU_ASSIGN_OR_RETURN (m_loop.beginIterationLabel, m_fragment->appendLabel());

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

    lyric_assembler::JumpTarget nextIterationTarget;
    TU_ASSIGN_OR_RETURN (nextIterationTarget, m_fragment->unconditionalJump());
    TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(nextIterationTarget, m_loop.beginIterationLabel));

    lyric_assembler::JumpLabel loopFinishedLabel;
    TU_ASSIGN_OR_RETURN (loopFinishedLabel, m_fragment->appendLabel());

    TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(m_loop.exitLoopTarget, loopFinishedLabel));

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

    TU_ASSIGN_OR_RETURN (m_loop->exitLoopTarget, m_fragment->jumpIfFalse());

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

    lyric_assembler::JumpTarget nextIteration;
    TU_ASSIGN_OR_RETURN (nextIteration, m_fragment->unconditionalJump());
    TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(nextIteration, m_iteration.topOfLoop));

    lyric_assembler::JumpLabel exitLoop;
    TU_ASSIGN_OR_RETURN (exitLoop, m_fragment->appendLabel());

    TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(m_iteration.predicateJump, exitLoop));

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

static tempo_utils::Result<lyric_common::TypeDef>
get_or_construct_iterator(
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem,
    lyric_assembler::ConceptSymbol *iteratorConcept,
    lyric_assembler::ConceptSymbol *iterableConcept,
    const lyric_common::TypeDef &generatorType,
    const lyric_common::TypeDef &targetType,
    lyric_assembler::CodeFragment *fragment)
{
    // if target type is specified then the iterator type must conform to Iterator[targetType]
    if (targetType.isValid()) {
        auto iteratorType = lyric_common::TypeDef::forConcrete(iteratorConcept->getSymbolUrl(), {targetType});

        // if generatorType implements iteratorType then we are done
        bool implementsIterator;
        TU_ASSIGN_OR_RETURN (implementsIterator, typeSystem->isImplementable(iteratorType, generatorType));
        if (implementsIterator)
            return iteratorType;

        // if generatorType doesn't implement Iterator then fall through
    }

    // otherwise check if generatorType implements Iterable[targetType]
    auto iterableType = lyric_common::TypeDef::forConcrete(iterableConcept->getSymbolUrl(), {targetType});
    bool implementsIterable;
    TU_ASSIGN_OR_RETURN (implementsIterable, typeSystem->isImplementable(iterableType, generatorType));
    if (!implementsIterable)
        return lyric_compiler::CompilerStatus::forCondition(lyric_compiler::CompilerCondition::kIncompatibleType,
            "generator type {} is not iterable", generatorType.toString());

    // resolve the Iterate impl method
    lyric_assembler::CallableInvoker invoker;
    TU_RETURN_IF_NOT_OK (iterableConcept->prepareAction("Iterate", iterableType, invoker));

    // invoke Iterate()
    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(invoker, {targetType}));

    return invoker.invoke(block, reifier, fragment);
}

tempo_utils::Status
lyric_compiler::ForBody::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto *fundamentalCache = driver->getFundamentalCache();
    auto *symbolCache = driver->getSymbolCache();
    auto *typeSystem = driver->getTypeSystem();

    m_iteration->generatorType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    // look up the Iterator concept symbol
    auto fundamentalIterator = fundamentalCache->getFundamentalUrl(
        lyric_assembler::FundamentalSymbol::Iterator);
    lyric_assembler::AbstractSymbol *iteratorSym;
    TU_ASSIGN_OR_RETURN (iteratorSym, symbolCache->getOrImportSymbol(fundamentalIterator));
    if (iteratorSym->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
        return CompilerStatus::forCondition(CompilerCondition::kInvalidSymbol,
            "invalid concept symbol {}", fundamentalIterator.toString());
    auto *iteratorConcept = cast_symbol_to_concept(iteratorSym);

    // look up the Iterable concept symbol
    auto fundamentalIterable = fundamentalCache->getFundamentalUrl(
        lyric_assembler::FundamentalSymbol::Iterable);
    lyric_assembler::AbstractSymbol *iterableSym;
    TU_ASSIGN_OR_RETURN (iterableSym, symbolCache->getOrImportSymbol(fundamentalIterable));
    if (iterableSym->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
        return CompilerStatus::forCondition(CompilerCondition::kInvalidSymbol,
            "invalid concept symbol {}", fundamentalIterator.toString());
    auto *iterableConcept = cast_symbol_to_concept(iterableSym);

    // put the iterator on the top of the stack. if the result of the generator is the iterator then this
    // call does not modify the stack. otherwise if the generator type implements Iterable then generator
    // will be swapped on the stack with the result of Iterate().
    lyric_common::TypeDef iteratorType;
    TU_ASSIGN_OR_RETURN (iteratorType, get_or_construct_iterator(
        block, typeSystem, iteratorConcept, iterableConcept,
        m_iteration->generatorType, m_iteration->targetType, m_fragment));

    // declare temp variable to store the iterator
    lyric_assembler::DataReference iteratorRef;
    TU_ASSIGN_OR_RETURN (iteratorRef, block->declareTemporary(iteratorType, /* isVariable= */ false));

    // store the iterator in the temp variable
    TU_RETURN_IF_NOT_OK (m_fragment->storeRef(iteratorRef, /* initialStore= */ true));

    // create a new block for the body of the loop
    m_iteration->forBlock = std::make_unique<lyric_assembler::BlockHandle>(
        block->blockProc(), block, block->blockState());
    auto *forBlock = m_iteration->forBlock.get();

    //
    TU_ASSIGN_OR_RETURN (m_iteration->topOfLoop, m_fragment->appendLabel());

    // declare the target variable which stores the value yielded from the iterator on each loop iteration
    lyric_assembler::DataReference targetRef;
    TU_ASSIGN_OR_RETURN (targetRef, forBlock->declareVariable(
        m_iteration->targetIdentifier, /* isHidden= */ true,
        m_iteration->targetType, /* isVariable= */ true));

    // push the iterator onto the stack, and invoke Valid() method
    TU_RETURN_IF_NOT_OK (m_fragment->loadRef(iteratorRef));

    // resolve Iterator.Valid() method
    lyric_assembler::CallableInvoker validInvoker;
    TU_RETURN_IF_NOT_OK (iteratorConcept->prepareAction("Valid", iteratorType, validInvoker));

    // invoke Valid method
    lyric_typing::CallsiteReifier validReifier(typeSystem);
    TU_RETURN_IF_NOT_OK (validReifier.initialize(validInvoker, {targetRef.typeDef}));

    lyric_common::TypeDef validReturnType;
    TU_ASSIGN_OR_RETURN (validReturnType, validInvoker.invoke(forBlock, validReifier, m_fragment));

    auto boolType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);

    bool isAssignable;
    TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(boolType, validReturnType));
    if (!isAssignable)
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "expected Valid method to return {}; found {}",
            boolType.toString(), validReturnType.toString());

    TU_ASSIGN_OR_RETURN (m_iteration->predicateJump, m_fragment->jumpIfFalse());

    // push the iterator onto the stack, and invoke Next() method
    TU_RETURN_IF_NOT_OK (m_fragment->loadRef(iteratorRef));

    // resolve Iterator.Next() method
    lyric_assembler::CallableInvoker nextInvoker;
    TU_RETURN_IF_NOT_OK (iteratorConcept->prepareAction("Next", iteratorType, nextInvoker));

    // invoke Next()
    lyric_typing::CallsiteReifier nextReifier(typeSystem);
    TU_RETURN_IF_NOT_OK (nextReifier.initialize(nextInvoker, {targetRef.typeDef}));
    lyric_common::TypeDef nextReturnType;
    TU_ASSIGN_OR_RETURN (nextReturnType, nextInvoker.invoke(forBlock, nextReifier, m_fragment));

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