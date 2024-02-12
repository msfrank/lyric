
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/code_builder.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/internal/compile_loop.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/callsite_reifier.h>

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

    auto boolType = block->blockState()->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);
    if (!typeSystem->isAssignable(boolType, testReturnType))
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

    // invoke the iterator expression
    auto forGenerator = walker.getChild(0);
    auto compileGeneratorResult = compile_expression(block, forGenerator, moduleEntry);
    if (compileGeneratorResult.isStatus())
        return compileGeneratorResult.getStatus();
    auto generatorType = compileGeneratorResult.getResult();

    // look up the Iterator class symbol
    auto fundamentalIterator = state->fundamentalCache()->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Iterator);
    auto *iteratorSym = state->symbolCache()->getSymbol(fundamentalIterator);
    if (iteratorSym == nullptr)
        block->throwAssemblerInvariant("missing class symbol {}", fundamentalIterator.toString());
    if (iteratorSym->getSymbolType() != lyric_assembler::SymbolType::CLASS)
        block->throwAssemblerInvariant("invalid class symbol {}", fundamentalIterator.toString());
    auto *iteratorClass = cast_symbol_to_class(iteratorSym);

    // synthesize the iterator type if needed
    lyric_common::TypeDef iteratorType;
    if (targetType.isValid()) {
        iteratorType = lyric_common::TypeDef::forConcrete(fundamentalIterator, {targetType});
    } else {
        if (generatorType.getType() != lyric_common::TypeDefType::Concrete)
            return block->logAndContinue(forGenerator,
                CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "invalid generator type {}", generatorType.toString());
        auto resultTypeParameters = generatorType.getConcreteArguments();
        if (resultTypeParameters.size() != 1)
            return block->logAndContinue(forGenerator,
                CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "invalid generator type {}", generatorType.toString());
        iteratorType = lyric_common::TypeDef::forConcrete(
            fundamentalIterator, std::vector<lyric_common::TypeDef>(
                resultTypeParameters.begin(), resultTypeParameters.end()));
        targetType = resultTypeParameters.front();
    }

//    auto iteratorType = iteratorResult.getResult();

    // validate that the generator expression conforms to the iterator type
    if (!typeSystem->isAssignable(iteratorType, generatorType))
        return block->logAndContinue(forGenerator,
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "expected generator expression to return {}; found {}", iteratorType.toString(), generatorType.toString());

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
    auto resolveValidMethodResult = iteratorClass->resolveMethod("valid", iteratorType);
    if (resolveValidMethodResult.isStatus())
        return resolveValidMethodResult.getStatus();
    auto valid = resolveValidMethodResult.getResult();

    // invoke valid method
    lyric_typing::CallsiteReifier validReifier(valid.getParameters(), valid.getRest(),
        valid.getTemplateUrl(), valid.getTemplateParameters(), valid.getTemplateArguments(),
        typeSystem);
    auto invokeValidResult = valid.invoke(&forBlock, validReifier);
    if (invokeValidResult.isStatus())
        return invokeValidResult.getStatus();

    auto boolType = state->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);
    if (!typeSystem->isAssignable(boolType, invokeValidResult.getResult()))
        forBlock.throwAssemblerInvariant("expected valid() method to return {}; found {}",
            boolType.toString(), invokeValidResult.getResult().toString());

    auto predicateJumpResult = code->jumpIfFalse();
    if (predicateJumpResult.isStatus())
        return predicateJumpResult.getStatus();
    auto predicateJump = predicateJumpResult.getResult();

    // push the iterator onto the stack, and invoke next() method
    status = forBlock.load(iterator);
    if (!status.isOk())
        return status;

    // resolve Iterator.next() method
    auto resolveNextMethodResult = iteratorClass->resolveMethod("next", iteratorType);
    if (resolveNextMethodResult.isStatus())
        return resolveNextMethodResult.getStatus();
    auto next = resolveNextMethodResult.getResult();

    // invoke next()
    lyric_typing::CallsiteReifier nextReifier(next.getParameters(), next.getRest(),
        next.getTemplateUrl(), next.getTemplateParameters(), next.getTemplateArguments(),
        typeSystem);
    auto invokeNextResult = next.invoke(&forBlock, nextReifier);
    if (invokeNextResult.isStatus())
        return invokeNextResult.getStatus();

    if (!typeSystem->isAssignable(targetType, invokeNextResult.getResult()))
        forBlock.throwAssemblerInvariant("expected next() method to return {}; found {}",
            targetType.toString(), invokeNextResult.getResult().toString());

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
