
#include <lyric_assembler/code_builder.h>
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

    auto *code = block->blockCode();
    lyric_assembler::BlockHandle whileBlock(block->blockProc(), code, block, block->blockState());

    auto topOfLoopResult = code->makeLabel();
    if (topOfLoopResult.isStatus())
        return topOfLoopResult.getStatus();
    auto topOfLoop = topOfLoopResult.getResult();

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
        return block->logAndContinue(whileTest,
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "expected test expression to return {}; found {}", boolType.toString(), testReturnType.toString());

    auto predicateJumpResult = code->jumpIfFalse();
    if (predicateJumpResult.isStatus())
        return predicateJumpResult.getStatus();
    auto predicateJump = predicateJumpResult.getResult();

    auto whileBody = walker.getChild(1);
    auto blockResult = compile_node(block, whileBody, moduleEntry);
    if (blockResult.isStatus())
        return blockResult.getStatus();

    // if block returns a value, then pop it off the top of the stack
    if (blockResult.getResult().isValid())
        code->popValue();

    auto status = code->jump(topOfLoop);
    if (!status.isOk())
        return status;

    auto exitLoopResult = code->makeLabel();
    if (exitLoopResult.isStatus())
        return exitLoopResult.getStatus();
    auto exitLoop = exitLoopResult.getResult();

    return code->patch(predicateJump, exitLoop);
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
    auto resolveIterateMethodResult = iterableConcept->resolveAction("Iterate", iterableType);
    if (resolveIterateMethodResult.isStatus())
        return resolveIterateMethodResult.getStatus();
    auto invoker = resolveIterateMethodResult.getResult();

    // invoke Iterate()
    lyric_typing::CallsiteReifier reifier(invoker.getParameters(), invoker.getRest(),
        invoker.getTemplateUrl(), invoker.getTemplateParameters(), {targetType}, typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize());

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
        tu_uint32 typeOffset;
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
        auto assignedType = walker.getNodeAtOffset(typeOffset);
        auto resolveTargetTypeResult = typeSystem->resolveAssignable(block, assignedType);
        if (resolveTargetTypeResult.isStatus())
            return resolveTargetTypeResult.getStatus();
        targetType = resolveTargetTypeResult.getResult();
    }

    // invoke the generator expression
    auto forGenerator = walker.getChild(0);
    auto compileGeneratorResult = compile_expression(block, forGenerator, moduleEntry);
    if (compileGeneratorResult.isStatus())
        return compileGeneratorResult.getStatus();
    auto generatorType = compileGeneratorResult.getResult();

    // look up the Iterator concept symbol
    auto fundamentalIterator = state->fundamentalCache()->getFundamentalUrl(
        lyric_assembler::FundamentalSymbol::Iterator);
    auto *iteratorSym = state->symbolCache()->getSymbol(fundamentalIterator);
    if (iteratorSym == nullptr)
        block->throwAssemblerInvariant("missing concept symbol {}", fundamentalIterator.toString());
    if (iteratorSym->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
        block->throwAssemblerInvariant("invalid concept symbol {}", fundamentalIterator.toString());
    auto *iteratorConcept = cast_symbol_to_concept(iteratorSym);

    // look up the Iterable concept symbol
    auto fundamentalIterable = state->fundamentalCache()->getFundamentalUrl(
        lyric_assembler::FundamentalSymbol::Iterable);
    auto *iterableSym = state->symbolCache()->getSymbol(fundamentalIterable);
    if (iterableSym == nullptr)
        block->throwAssemblerInvariant("missing concept symbol {}", fundamentalIterator.toString());
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
    auto declareTempResult = block->declareTemporary(iteratorType, lyric_parser::BindingType::VALUE);
    if (declareTempResult.isStatus())
        return declareTempResult.getStatus();
    auto iterator = declareTempResult.getResult();

    // store the iterator in the temp variable
    auto status = block->store(iterator);
    if (!status.isOk())
        return status;

    auto *code = block->blockCode();

    // create a new block for the body of the loop
    lyric_assembler::BlockHandle forBlock(block->blockProc(), code, block, state);

    auto topOfLoopResult = code->makeLabel();
    if (topOfLoopResult.isStatus())
        return topOfLoopResult.getStatus();
    auto topOfLoop = topOfLoopResult.getResult();

    // declare the target variable which stores the value yielded from the iterator on each loop iteration
    auto declareTargetResult = forBlock.declareVariable(
        identifier, targetType, lyric_parser::BindingType::VARIABLE);
    if (declareTargetResult.isStatus())
        return declareTargetResult.getStatus();
    auto target = declareTargetResult.getResult();

    // push the iterator onto the stack, and invoke valid() method
    status = forBlock.load(iterator);
    if (!status.isOk())
        return status;

    // resolve Iterator.valid() method
    auto resolveValidMethodResult = iteratorConcept->resolveAction("Valid", iteratorType);
    if (resolveValidMethodResult.isStatus())
        return resolveValidMethodResult.getStatus();
    auto valid = resolveValidMethodResult.getResult();

    // invoke valid method
    lyric_typing::CallsiteReifier validReifier(valid.getParameters(), valid.getRest(),
        valid.getTemplateUrl(), valid.getTemplateParameters(), {target.typeDef}, typeSystem);
    TU_RETURN_IF_NOT_OK (validReifier.initialize());
    auto invokeValidResult = valid.invoke(&forBlock, validReifier);
    if (invokeValidResult.isStatus())
        return invokeValidResult.getStatus();
    auto validReturnType = invokeValidResult.getResult();

    auto boolType = state->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);
    bool isAssignable;

    TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(boolType, validReturnType));
    if (!isAssignable)
        forBlock.throwAssemblerInvariant("expected Valid method to return {}; found {}",
            boolType.toString(), validReturnType.toString());

    auto predicateJumpResult = code->jumpIfFalse();
    if (predicateJumpResult.isStatus())
        return predicateJumpResult.getStatus();
    auto predicateJump = predicateJumpResult.getResult();

    // push the iterator onto the stack, and invoke next() method
    status = forBlock.load(iterator);
    if (!status.isOk())
        return status;

    // resolve Iterator.next() method
    auto resolveNextMethodResult = iteratorConcept->resolveAction("Next", iteratorType);
    if (resolveNextMethodResult.isStatus())
        return resolveNextMethodResult.getStatus();
    auto next = resolveNextMethodResult.getResult();

    // invoke next()
    lyric_typing::CallsiteReifier nextReifier(next.getParameters(), next.getRest(),
        next.getTemplateUrl(), next.getTemplateParameters(), {target.typeDef}, typeSystem);
    TU_RETURN_IF_NOT_OK (nextReifier.initialize());
    auto invokeNextResult = next.invoke(&forBlock, nextReifier);
    if (invokeNextResult.isStatus())
        return invokeNextResult.getStatus();
    auto nextReturnType = invokeNextResult.getResult();

    TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(targetType, nextReturnType));
    if (!isAssignable)
        forBlock.throwAssemblerInvariant("expected Next method to return {}; found {}",
            targetType.toString(), nextReturnType.toString());

    // store the next value in the target variable
    status = forBlock.store(target);
    if (!status.isOk())
        return status;

    // execute the body of the for loop
    auto forBody = walker.getChild(1);
    auto blockResult = compile_node(&forBlock, forBody, moduleEntry);
    if (blockResult.isStatus())
        return blockResult.getStatus();

    // if block returns a value, then pop it off the top of the stack
    if (blockResult.getResult().isValid())
        code->popValue();

    status = code->jump(topOfLoop);
    if (!status.isOk())
        return status;

    auto exitLoopResult = code->makeLabel();
    if (exitLoopResult.isStatus())
        return exitLoopResult.getStatus();
    auto exitLoop = exitLoopResult.getResult();

    return code->patch(predicateJump, exitLoop);
}
