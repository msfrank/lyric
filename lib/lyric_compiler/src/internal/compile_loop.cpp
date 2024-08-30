
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/internal/compile_loop.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/callsite_reifier.h>
#include <lyric_typing/type_system.h>

tempo_utils::Status
lyric_compiler::internal::compile_while(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();

    moduleEntry.checkClassAndChildCountOrThrow(walker, lyric_schema::kLyricAstWhileClass, 2);

    auto *blockCode = block->blockCode();
    auto *fragment = blockCode->rootFragment();
    lyric_assembler::BlockHandle whileBlock(block->blockProc(), blockCode, block, block->blockState());

    lyric_assembler::JumpLabel topOfLoop;
    TU_ASSIGN_OR_RETURN (topOfLoop, fragment->appendLabel());

    auto whileTest = walker.getChild(0);
    auto testResult = compile_expression(&whileBlock, whileTest, moduleEntry);
    if (testResult.isStatus())
        return testResult.getStatus();
    auto testReturnType = testResult.getResult();

    auto boolType = block->blockState()->fundamentalCache()->getFundamentalType(
        lyric_assembler::FundamentalSymbol::Bool);

    bool isAssignable;
    TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(boolType, testReturnType));
    if (!isAssignable)
        return moduleEntry.logAndContinue(whileTest.getLocation(),
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "expected test expression to return {}; found {}", boolType.toString(), testReturnType.toString());

    lyric_assembler::JumpTarget predicateJump;
    TU_ASSIGN_OR_RETURN (predicateJump, fragment->jumpIfFalse());

    auto whileBody = walker.getChild(1);
    lyric_common::TypeDef resultType;
    TU_ASSIGN_OR_RETURN (resultType, compile_node(block, whileBody, moduleEntry));

    // if block returns a value, then pop it off the top of the stack
    TU_ASSERT (resultType.isValid());
    if (resultType != lyric_common::TypeDef::noReturn()) {
        TU_RETURN_IF_NOT_OK (fragment->popValue());
    }

    lyric_assembler::JumpTarget nextIteration;
    TU_ASSIGN_OR_RETURN (nextIteration, fragment->unconditionalJump());
    TU_RETURN_IF_NOT_OK (fragment->patchTarget(nextIteration, topOfLoop));

    lyric_assembler::JumpLabel exitLoop;
    TU_ASSIGN_OR_RETURN (exitLoop, fragment->appendLabel());

    return fragment->patchTarget(predicateJump, exitLoop);
}

static tempo_utils::Result<lyric_common::TypeDef>
get_or_construct_iterator(
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem,
    lyric_assembler::ConceptSymbol *iteratorConcept,
    lyric_assembler::ConceptSymbol *iterableConcept,
    const lyric_common::TypeDef &generatorType,
    const lyric_common::TypeDef &targetType)
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
        return block->logAndContinue(lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "generator type {} is not iterable", generatorType.toString());

    // resolve the Iterate impl method
    lyric_assembler::CallableInvoker invoker;
    TU_RETURN_IF_NOT_OK (iterableConcept->prepareAction("Iterate", iterableType, invoker));

    // invoke Iterate()
    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(invoker, {targetType}));
    return invoker.invoke(block, reifier);
}

tempo_utils::Status
lyric_compiler::internal::compile_for(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();
    auto *state = moduleEntry.getState();

    moduleEntry.checkClassAndChildCountOrThrow(walker, lyric_schema::kLyricAstForClass, 2);

    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // resolve the target type if it was specified
    lyric_common::TypeDef targetType;
    if (walker.hasAttr(lyric_parser::kLyricAstTypeOffset)) {
        lyric_parser::NodeWalker typeNode;
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeNode);
        lyric_typing::TypeSpec targetSpec;
        TU_ASSIGN_OR_RETURN (targetSpec, typeSystem->parseAssignable(block, typeNode));
        TU_ASSIGN_OR_RETURN (targetType, typeSystem->resolveAssignable(block, targetSpec));
    }

    // invoke the generator expression
    auto forGenerator = walker.getChild(0);
    lyric_common::TypeDef generatorType;
    TU_ASSIGN_OR_RETURN (generatorType, compile_expression(block, forGenerator, moduleEntry));

    // look up the Iterator concept symbol
    auto fundamentalIterator = state->fundamentalCache()->getFundamentalUrl(
        lyric_assembler::FundamentalSymbol::Iterator);
    lyric_assembler::AbstractSymbol *iteratorSym;
    TU_ASSIGN_OR_RETURN (iteratorSym, state->symbolCache()->getOrImportSymbol(fundamentalIterator));
    if (iteratorSym->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
        block->throwAssemblerInvariant("invalid concept symbol {}", fundamentalIterator.toString());
    auto *iteratorConcept = cast_symbol_to_concept(iteratorSym);

    // look up the Iterable concept symbol
    auto fundamentalIterable = state->fundamentalCache()->getFundamentalUrl(
        lyric_assembler::FundamentalSymbol::Iterable);
    lyric_assembler::AbstractSymbol *iterableSym;
    TU_ASSIGN_OR_RETURN (iterableSym, state->symbolCache()->getOrImportSymbol(fundamentalIterable));
    if (iterableSym->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
        block->throwAssemblerInvariant("invalid concept symbol {}", fundamentalIterator.toString());
    auto *iterableConcept = cast_symbol_to_concept(iterableSym);

    // put the iterator on the top of the stack. if the result of the generator is the iterator then this
    // call does not modify the stack. otherwise if the generator type implements Iterable then generator
    // will be swapped on the stack with the result of Iterate().
    lyric_common::TypeDef iteratorType;
    TU_ASSIGN_OR_RETURN (iteratorType, get_or_construct_iterator(
        block, typeSystem, iteratorConcept, iterableConcept, generatorType, targetType));

    // declare temp variable to store the iterator
    lyric_assembler::DataReference iterator;
    TU_ASSIGN_OR_RETURN (iterator, block->declareTemporary(iteratorType, /* isVariable= */ false));

    // store the iterator in the temp variable
    TU_RETURN_IF_NOT_OK (block->store(iterator, /* initialStore= */ true));

    auto *blockCode = block->blockCode();
    auto *fragment = blockCode->rootFragment();

    // create a new block for the body of the loop
    lyric_assembler::BlockHandle forBlock(block->blockProc(), blockCode, block, state);
    lyric_assembler::JumpLabel topOfLoop;
    TU_ASSIGN_OR_RETURN (topOfLoop, fragment->appendLabel());

    // declare the target variable which stores the value yielded from the iterator on each loop iteration
    lyric_assembler::DataReference target;
    TU_ASSIGN_OR_RETURN (target, forBlock.declareVariable(
        identifier, lyric_object::AccessType::Private, targetType, /* isVariable= */ true));

    // push the iterator onto the stack, and invoke valid() method
    TU_RETURN_IF_NOT_OK (forBlock.load(iterator));

    // resolve Iterator.valid() method
    lyric_assembler::CallableInvoker validInvoker;
    TU_RETURN_IF_NOT_OK (iteratorConcept->prepareAction("Valid", iteratorType, validInvoker));

    // invoke valid method
    lyric_typing::CallsiteReifier validReifier(typeSystem);
    TU_RETURN_IF_NOT_OK (validReifier.initialize(validInvoker, {target.typeDef}));

    lyric_common::TypeDef validReturnType;
    TU_ASSIGN_OR_RETURN (validReturnType, validInvoker.invoke(&forBlock, validReifier));

    auto boolType = state->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);

    bool isAssignable;
    TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(boolType, validReturnType));
    if (!isAssignable)
        forBlock.throwAssemblerInvariant("expected Valid method to return {}; found {}",
            boolType.toString(), validReturnType.toString());

    lyric_assembler::JumpTarget predicateJump;
    TU_ASSIGN_OR_RETURN (predicateJump, fragment->jumpIfFalse());

    // push the iterator onto the stack, and invoke next() method
    TU_RETURN_IF_NOT_OK (forBlock.load(iterator));

    // resolve Iterator.next() method
    lyric_assembler::CallableInvoker nextInvoker;
    TU_RETURN_IF_NOT_OK (iteratorConcept->prepareAction("Next", iteratorType, nextInvoker));

    // invoke next()
    lyric_typing::CallsiteReifier nextReifier(typeSystem);
    TU_RETURN_IF_NOT_OK (nextReifier.initialize(nextInvoker, {target.typeDef}));
    lyric_common::TypeDef nextReturnType;
    TU_ASSIGN_OR_RETURN (nextReturnType, nextInvoker.invoke(&forBlock, nextReifier));

    TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(targetType, nextReturnType));
    if (!isAssignable)
        forBlock.throwAssemblerInvariant("expected Next method to return {}; found {}",
            targetType.toString(), nextReturnType.toString());

    // store the next value in the target variable
    TU_RETURN_IF_NOT_OK (forBlock.store(target));

    // execute the body of the for loop
    auto forBody = walker.getChild(1);
    lyric_common::TypeDef blockReturnType;
    TU_ASSIGN_OR_RETURN (blockReturnType, compile_node(&forBlock, forBody, moduleEntry));

    // if block returns a value, then pop it off the top of the stack
    if (blockReturnType.getType() != lyric_common::TypeDefType::NoReturn) {
        TU_RETURN_IF_NOT_OK (fragment->popValue());
    }

    lyric_assembler::JumpTarget nextIteration;
    TU_ASSIGN_OR_RETURN (nextIteration, fragment->unconditionalJump());
    TU_RETURN_IF_NOT_OK (fragment->patchTarget(nextIteration, topOfLoop));

    lyric_assembler::JumpLabel exitLoop;
    TU_ASSIGN_OR_RETURN (exitLoop, fragment->appendLabel());

    return fragment->patchTarget(predicateJump, exitLoop);
}
