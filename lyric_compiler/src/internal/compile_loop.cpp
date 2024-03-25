
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

static tempo_utils::Result<lyric_common::TypeDef>
get_iterator_type(
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem,
    lyric_assembler::ConceptSymbol *iteratorConcept,
    const lyric_common::TypeDef &generatorType,
    const lyric_common::TypeDef &targetType)
{
    // if target type is specified then verify that the generator type implements Iterator for the target type
    if (targetType.isValid()) {
        if (targetType.getType() != lyric_common::TypeDefType::Concrete)
            return block->logAndContinue(lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "incompatible generator type {}", generatorType.toString());

        // if the generator type is Iterator then verify that the generator type has a
        // single param and it matches the target type
        if (generatorType.getConcreteUrl() == iteratorConcept->getSymbolUrl()) {
            auto typeArguments = generatorType.getConcreteArguments();
            if (typeArguments.size() != 1)
                return block->logAndContinue(lyric_compiler::CompilerCondition::kTypeError,
                    tempo_tracing::LogSeverity::kError,
                    "invalid generator type {}; expected a single type argument for Iterator",
                    generatorType.toString());
            auto &typeArg0 = typeArguments.front();
            if (!typeSystem->isAssignable(targetType, typeArg0))
                return block->logAndContinue(lyric_compiler::CompilerCondition::kIncompatibleType,
                    tempo_tracing::LogSeverity::kError,
                    "invalid generator type {}; type argument {} is not assignable to target type {}",
                    generatorType.toString(), typeArg0.toString(), targetType.toString());
            return generatorType;
        }

        auto iteratorType = lyric_common::TypeDef::forConcrete(iteratorConcept->getSymbolUrl(), {targetType});
        bool isImplementable;
        TU_ASSIGN_OR_RETURN (isImplementable, typeSystem->isImplementable(iteratorType, generatorType));
        if (!isImplementable)
            return block->logAndContinue(lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "generator type {} does not implement type {}",
                generatorType.toString(), iteratorType.toString());
        return iteratorType;
    }

    return block->logAndContinue(lyric_compiler::CompilerCondition::kCompilerInvariant,
        tempo_tracing::LogSeverity::kError,
        "iterator type couldn't be determined from generator type {}", generatorType.toString());
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

    // look up the Iterator concept symbol
    auto fundamentalIterator = state->fundamentalCache()->getFundamentalUrl(
        lyric_assembler::FundamentalSymbol::Iterator);
    auto *iteratorSym = state->symbolCache()->getSymbol(fundamentalIterator);
    if (iteratorSym == nullptr)
        block->throwAssemblerInvariant("missing concept symbol {}", fundamentalIterator.toString());
    if (iteratorSym->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
        block->throwAssemblerInvariant("invalid concept symbol {}", fundamentalIterator.toString());
    auto *iteratorConcept = cast_symbol_to_concept(iteratorSym);

    // determine the iterator type
    lyric_common::TypeDef iteratorType;
    TU_ASSIGN_OR_RETURN (iteratorType, get_iterator_type(
        block, typeSystem, iteratorConcept, generatorType, targetType));

//    // validate that the generator expression conforms to the iterator type
//    if (!typeSystem->isAssignable(iteratorType, generatorType))
//        return block->logAndContinue(forGenerator, lyric_compiler::CompilerCondition::kIncompatibleType,
//            tempo_tracing::LogSeverity::kError,
//            "expected generator expression to return {}; found {}", iteratorType.toString(), generatorType.toString());

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
        valid.getTemplateUrl(), valid.getTemplateParameters(), valid.getTemplateArguments(),
        typeSystem);
    auto invokeValidResult = valid.invoke(&forBlock, validReifier);
    if (invokeValidResult.isStatus())
        return invokeValidResult.getStatus();

    auto boolType = state->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);
    if (!typeSystem->isAssignable(boolType, invokeValidResult.getResult()))
        forBlock.throwAssemblerInvariant("expected Valid method to return {}; found {}",
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
    auto resolveNextMethodResult = iteratorConcept->resolveAction("Next", iteratorType);
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
        forBlock.throwAssemblerInvariant("expected Next method to return {}; found {}",
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
