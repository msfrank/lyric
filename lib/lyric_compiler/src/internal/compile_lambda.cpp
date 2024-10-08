
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/internal/compile_block.h>
#include <lyric_compiler/internal/compile_lambda.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_compiler/internal/compile_initializer.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/callsite_reifier.h>

tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::internal::compile_lambda(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry,
    const std::vector<lyric_object::TemplateParameter> &templateParameters)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT(walker.isValid());
    auto *state = moduleEntry.getState();
    auto *typeSystem = moduleEntry.getTypeSystem();

    moduleEntry.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstLambdaClass, 2);

    /*
     * step 1: declare the lambda call
     */

    // name of the lambda function is "$lambda" + the current count of locals in the enclosing proc
    auto lambdaName = absl::StrCat("$lambda", block->blockProc()->numLocals());

    // parse the return type
    lyric_parser::NodeWalker typeNode;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeNode);
    lyric_typing::TypeSpec returnSpec;
    TU_ASSIGN_OR_RETURN (returnSpec, typeSystem->parseAssignable(block, typeNode));

    // parse the parameter list
    auto pack = walker.getChild(0);
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(block, pack));

    // check for initializers
    if (!packSpec.initializers.empty()) {
        for (const auto &p: packSpec.listParameterSpec) {
            if (!p.init.isEmpty())
                return moduleEntry.logAndContinue(p.init.getValue().getLocation(),
                    lyric_compiler::CompilerCondition::kSyntaxError,
                    tempo_tracing::LogSeverity::kError,
                    "list parameter '{}' has unexpected initializer", p.name);
        }
    }

    // check for named parameters
    if (!packSpec.namedParameterSpec.empty()) {
        auto &p = packSpec.namedParameterSpec.front();
        return moduleEntry.logAndContinue(p.node.getLocation(),
            lyric_compiler::CompilerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "unexpected named parameter '{}'", p.name);
    }

    // check for ctx parameters
    if (!packSpec.ctxParameterSpec.empty()) {
        auto &p = packSpec.ctxParameterSpec.front();
        return moduleEntry.logAndContinue(p.node.getLocation(),
            lyric_compiler::CompilerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "unexpected ctx parameter '{}'", p.name);
    }

    // check for rest parameter
    if (!packSpec.restParameterSpec.isEmpty()) {
        auto p = packSpec.restParameterSpec.getValue();
        return moduleEntry.logAndContinue(p.node.getLocation(),
            lyric_compiler::CompilerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "unexpected rest parameter");
    }

    // declare the function call
    lyric_assembler::CallSymbol *lambdaCallSymbol;
    TU_ASSIGN_OR_RETURN (lambdaCallSymbol, block->declareFunction(lambdaName,
        lyric_object::AccessType::Public, {}));

    auto *resolver = lambdaCallSymbol->callResolver();

    // resolve the parameter pack
    lyric_assembler::ParameterPack lambdaParameterPack;
    TU_ASSIGN_OR_RETURN (lambdaParameterPack, typeSystem->resolvePack(resolver, packSpec));

    // resolve the return type
    lyric_common::TypeDef lambdaReturnType;
    TU_ASSIGN_OR_RETURN (lambdaReturnType, typeSystem->resolveAssignable(block, returnSpec));

    // define the lambda
    lyric_assembler::ProcHandle *lambdaProcHandle;
    TU_ASSIGN_OR_RETURN (lambdaProcHandle, lambdaCallSymbol->defineCall(lambdaParameterPack, lambdaReturnType));

    // compile the lambda body
    auto lambdaBody = walker.getChild(1);
    lyric_common::TypeDef lambdaBodyType;
    TU_ASSIGN_OR_RETURN (lambdaBodyType, compile_block(lambdaProcHandle->procBlock(), lambdaBody, moduleEntry));

    auto *lambdaProcCode = lambdaProcHandle->procCode();
    auto *lambdaFragment = lambdaProcCode->rootFragment();

    // add return instruction
    TU_RETURN_IF_NOT_OK (lambdaFragment->returnToCaller());

    // validate that body returns the expected type
    bool isReturnable;
    TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(lambdaReturnType, lambdaBodyType));
    if (!isReturnable)
        return moduleEntry.logAndContinue(lambdaBody.getLocation(),
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "body does not match return type {}", lambdaReturnType.toString());

    // validate that each exit returns the expected type
    for (auto it = lambdaProcHandle->exitTypesBegin(); it != lambdaProcHandle->exitTypesEnd(); it++) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(lambdaReturnType, *it));
        if (!isReturnable)
            return moduleEntry.logAndContinue(lambdaBody.getLocation(),
                lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "body does not match return type {}", lambdaReturnType.toString());
    }

    /*
     * step 2: resolve the Closure class
     */

    // resolve the closure class address
    auto arity = packSpec.listParameterSpec.size();
    auto lambdaFunctionUrl = block->blockState()->fundamentalCache()->getFunctionUrl(arity);
    if (!lambdaFunctionUrl.isValid())
        block->throwAssemblerInvariant("lambda arity is too large");

    lyric_assembler::AbstractSymbol *closureSym;
    TU_ASSIGN_OR_RETURN (closureSym, block->blockState()->symbolCache()->getOrImportSymbol(lambdaFunctionUrl));
    if (closureSym->getSymbolType() != lyric_assembler::SymbolType::CLASS)
        block->throwAssemblerInvariant("invalid class symbol {}", lambdaFunctionUrl.toString());

    auto *closureClass = cast_symbol_to_class(closureSym);

    /*
     * step 3: declare the lambda builder
     */

    // return type is a Function with the arity specified by the count of lambda parameters
    auto builderTypePath = lambdaFunctionUrl.getSymbolPath().getPath();
    auto builderTypeLocation = lambdaFunctionUrl.getModuleLocation().toUrl();
    std::vector<lyric_common::TypeDef> builderTypeArguments;
    builderTypeArguments.push_back(lambdaReturnType);
    for (const auto &p : lambdaParameterPack.listParameters) {
        builderTypeArguments.push_back(p.typeDef);
    }
    auto builderReturnType = lyric_common::TypeDef::forConcrete(lambdaFunctionUrl, builderTypeArguments);

    // name of the builder function is the lambda function name + "$builder"
    auto builderName = absl::StrCat(lambdaName + "$lambda");

    // declare the lambda builder function
    lyric_assembler::CallSymbol *builderCallSymbol;
    TU_ASSIGN_OR_RETURN (builderCallSymbol, block->declareFunction(builderName, lyric_object::AccessType::Public, {}));

    lyric_assembler::ProcHandle *builderProcHandle;
    TU_ASSIGN_OR_RETURN (builderProcHandle, builderCallSymbol->defineCall({}, builderReturnType));

    // transfer the lexicals from the lambda to the builder
    for (auto lexical = lambdaProcHandle->lexicalsBegin(); lexical != lambdaProcHandle->lexicalsEnd(); lexical++) {
        builderProcHandle->allocateLexical(lexical->lexicalTarget, lexical->targetOffset, lexical->activationCall);
    }

    auto *builderProcCode = builderProcHandle->procCode();
    auto *builderFragment = builderProcCode->rootFragment();

    // invoke the closure ctor
    lyric_assembler::ConstructableInvoker closureCtor;
    TU_RETURN_IF_NOT_OK (closureClass->prepareCtor(closureCtor));

    lyric_typing::CallsiteReifier closureReifier(typeSystem);
    TU_RETURN_IF_NOT_OK (closureReifier.initialize(closureCtor, builderTypeArguments));

    auto *blockCode = block->blockCode();
    auto *fragment = blockCode->rootFragment();

    // push the proc descriptor onto the top of the stack as first positional arg
    TU_RETURN_IF_NOT_OK (fragment->loadDescriptor(lambdaCallSymbol));
    auto callType = state->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Call);
    TU_RETURN_IF_NOT_OK (closureReifier.reifyNextArgument(callType));

    TU_RETURN_IF_STATUS (closureCtor.invokeNew(builderProcHandle->procBlock(), closureReifier, 0));

    // add return instruction
    TU_RETURN_IF_NOT_OK (builderFragment->returnToCaller());

    /*
     * step 4: invoke the lambda builder
     */

    lyric_assembler::CallableInvoker builderInvoker;
    TU_RETURN_IF_NOT_OK (block->prepareFunction(builderName, builderInvoker));

    lyric_typing::CallsiteReifier builderReifier(typeSystem);
    TU_RETURN_IF_NOT_OK (builderReifier.initialize(builderInvoker));

    // the return value of the lambda builder is the lambda closure
    return builderInvoker.invoke(block, builderReifier);
}
