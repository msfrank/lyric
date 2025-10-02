
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/lambda_utils.h>

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_compiler::define_lambda_builder(
    lyric_assembler::CallSymbol *lambdaCall,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::FundamentalCache *fundamentalCache,
    lyric_assembler::SymbolCache *symbolCache,
    lyric_typing::TypeSystem *typeSystem)
{
    auto arity = lambdaCall->numParameters();
    auto lambdaFunctionUrl = fundamentalCache->getFunctionUrl(arity);
    if (!lambdaFunctionUrl.isValid())
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "lambda arity is too large");

    lyric_assembler::AbstractSymbol *closureSym;
    TU_ASSIGN_OR_RETURN (closureSym, symbolCache->getOrImportSymbol(lambdaFunctionUrl));
    if (closureSym->getSymbolType() != lyric_assembler::SymbolType::CLASS)
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "invalid class symbol {}", lambdaFunctionUrl.toString());

    auto *closureClass = cast_symbol_to_class(closureSym);

    // return type is a Function with the arity specified by the count of lambda parameters
    auto builderTypePath = lambdaFunctionUrl.getSymbolPath().getPath();
    auto builderTypeLocation = lambdaFunctionUrl.getModuleLocation().toUrl();

    std::vector<lyric_common::TypeDef> builderTypeArguments;
    builderTypeArguments.push_back(lambdaCall->getReturnType());
    for (auto it = lambdaCall->listPlacementBegin(); it != lambdaCall->listPlacementEnd(); it++) {
        builderTypeArguments.push_back(it->typeDef);
    }

    lyric_common::TypeDef builderReturnType;
    TU_ASSIGN_OR_RETURN (builderReturnType, lyric_common::TypeDef::forConcrete(
        lambdaFunctionUrl, builderTypeArguments));

    // name of the builder function is the lambda function name + "$builder"
    auto builderName = absl::StrCat(lambdaCall->getName() + "$builder");

    // declare the lambda builder function
    lyric_assembler::CallSymbol *builderCall;
    TU_ASSIGN_OR_RETURN (builderCall, block->declareFunction(builderName, /* isHidden= */ false, {}));

    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, builderCall->defineCall({}, builderReturnType));

    // transfer the lexicals from the lambda to the builder
    auto *lambdaProcHandle = lambdaCall->callProc();
    for (auto lexical = lambdaProcHandle->lexicalsBegin(); lexical != lambdaProcHandle->lexicalsEnd(); lexical++) {
        procHandle->allocateLexical(lexical->lexicalTarget, lexical->targetOffset, lexical->activationCall);
    }

    auto *fragment = procHandle->procFragment();

    // invoke the closure ctor
    lyric_assembler::ConstructableInvoker closureCtor;
    TU_RETURN_IF_NOT_OK (closureClass->prepareCtor(lyric_object::kCtorSpecialSymbol, closureCtor));

    lyric_typing::CallsiteReifier closureReifier(typeSystem);
    TU_RETURN_IF_NOT_OK (closureReifier.initialize(closureCtor, builderTypeArguments));

    // push the proc descriptor onto the top of the stack as first positional arg
    TU_RETURN_IF_NOT_OK (fragment->loadDescriptor(lambdaCall));
    auto callType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Call);
    TU_RETURN_IF_NOT_OK (closureReifier.reifyNextArgument(callType));

    TU_RETURN_IF_STATUS (closureCtor.invokeNew(procHandle->procBlock(), closureReifier, fragment, /* flags= */ 0));

    // add return instruction
    TU_RETURN_IF_NOT_OK (fragment->returnToCaller());

    return builderCall;
}
