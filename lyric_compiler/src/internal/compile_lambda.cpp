
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
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

    // name of the call is "$lambda" + the current count of locals in the enclosing proc
    auto lambdaName = absl::StrCat("$lambda", block->blockProc()->numLocals());

    // get function return type
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    auto parseAssignableResult = typeSystem->parseAssignable(block, type);
    if (parseAssignableResult.isStatus())
        return parseAssignableResult.getStatus();
    auto returnSpec = parseAssignableResult.getResult();

    // compile the parameter list
    auto pack = walker.getChild(0);
    auto parsePackResult = typeSystem->parsePack(block, pack);
    if (parsePackResult.isStatus())
        return parsePackResult.getStatus();
    auto packSpec = parsePackResult.getResult();

    for (const auto &p : packSpec.parameterSpec) {
        if (!p.label.empty())
            block->throwSyntaxError(pack, "named parameters are not supported for lambda");
        if (!p.init.isEmpty())
            block->throwSyntaxError(pack, "default initialization is not supported for lambda");
    }
    if (!packSpec.ctxSpec.empty())
        block->throwSyntaxError(pack, "ctx parameters are not supported for lambda");

    // declare the lambda
    auto declareLambdaResult = block->declareFunction(lambdaName,
        packSpec.parameterSpec, packSpec.restSpec, packSpec.ctxSpec,
        returnSpec, lyric_object::AccessType::Public, {});
    if (declareLambdaResult.isStatus())
        return declareLambdaResult.getStatus();
    auto lambdaUrl = declareLambdaResult.getResult();
    auto *lambdaSym = block->blockState()->symbolCache()->getSymbol(lambdaUrl);
    if (lambdaSym == nullptr)
        block->throwAssemblerInvariant("missing call symbol {}", lambdaUrl.toString());
    if (lambdaSym->getSymbolType() != lyric_assembler::SymbolType::CALL)
        block->throwAssemblerInvariant("invalid call symbol {}", lambdaUrl.toString());
    auto *lambdaCall = cast_symbol_to_call(lambdaSym);

    // compile the lambda body
    auto lambdaBody = walker.getChild(1);
    auto *lambdaProc = lambdaCall->callProc();
    auto compileLambdaBodyResult = compile_block(lambdaProc->procBlock(), lambdaBody, moduleEntry);
    if (compileLambdaBodyResult.isStatus())
        return compileLambdaBodyResult.getStatus();
    auto lambdaBodyType = compileLambdaBodyResult.getResult();

    // add return instruction
    auto status = lambdaProc->procCode()->writeOpcode(lyric_object::Opcode::OP_RETURN);
    if (!status.isOk())
        return status;

    bool isReturnable;

    // validate that body returns the expected type
    TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(lambdaCall->getReturnType(), lambdaBodyType));
    if (!isReturnable)
        return block->logAndContinue(lambdaBody,
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "body does not match return type {}", lambdaCall->getReturnType().toString());

    // validate that each exit returns the expected type
    for (const auto &exitType : lambdaCall->listExitTypes()) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(lambdaCall->getReturnType(), exitType));
        if (!isReturnable)
            return block->logAndContinue(lambdaBody,
                lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "body does not match return type {}", lambdaCall->getReturnType().toString());
    }

    /*
     * step 2: resolve the Closure class
     */

    // resolve the closure class address
    auto arity = packSpec.parameterSpec.size();
    auto lambdaFunctionUrl = block->blockState()->fundamentalCache()->getFunctionUrl(arity);
    if (!lambdaFunctionUrl.isValid())
        block->throwAssemblerInvariant("lambda arity is too large");

    auto *closureSym = state->symbolCache()->getSymbol(lambdaFunctionUrl);
    if (closureSym == nullptr)
        block->throwAssemblerInvariant("missing class symbol {}", lambdaFunctionUrl.toString());
    if (closureSym->getSymbolType() != lyric_assembler::SymbolType::CLASS)
        block->throwAssemblerInvariant("invalid class symbol {}", lambdaFunctionUrl.toString());

    closureSym->touch();

    auto *closureClass = cast_symbol_to_class(closureSym);

    /*
     * step 3: declare the lambda assembly
     */

    // return type is a Function with the arity specified by the count of lambda parameters
    auto builderTypePath = lambdaFunctionUrl.getSymbolPath().getPath();
    auto builderTypeLocation = lambdaFunctionUrl.getAssemblyLocation().toUrl();
    std::vector<lyric_parser::Assignable> builderTypeArguments;
    builderTypeArguments.push_back(returnSpec);
    for (const auto &p : packSpec.parameterSpec) {
        builderTypeArguments.push_back(p.type);
    }
    auto builderReturnSpec = lyric_parser::Assignable::forSingular(lambdaFunctionUrl, builderTypeArguments);

    // resolve the lambda type, which is needed as a type parameter to the Closure ctor param
    // it is safe to resolve via the spec because the absolute location is specified
    auto resolveLambdaTypeResult = block->resolveAssignable(builderReturnSpec);
    if (resolveLambdaTypeResult.isStatus())
        return resolveLambdaTypeResult.getStatus();
    auto builderReturnType = resolveLambdaTypeResult.getResult();

    // declare the lambda assembly
    auto declareBuilderResult = block->declareFunction(lambdaName + "$builder",
        {}, {}, {}, builderReturnSpec,
        lyric_object::AccessType::Public, {});
    if (declareBuilderResult.isStatus())
        return declareBuilderResult.getStatus();
    auto builderUrl = declareBuilderResult.getResult();
    auto *builderSym = block->blockState()->symbolCache()->getSymbol(builderUrl);
    if (builderSym == nullptr)
        block->throwAssemblerInvariant("missing call symbol {}", builderUrl.toString());
    if (builderSym->getSymbolType() != lyric_assembler::SymbolType::CALL)
        block->throwAssemblerInvariant("invalid call symbol {}", builderUrl.toString());
    auto *builderCall = cast_symbol_to_call(builderSym);

    // transfer the lexicals from the lambda to the assembly
    auto *builderProc = builderCall->callProc();
    for (auto lexical = lambdaProc->lexicalsBegin(); lexical != lambdaProc->lexicalsEnd(); lexical++) {
        builderProc->allocateLexical(lexical->lexicalTarget, lexical->targetOffset, lexical->activationCall);
    }

    auto *builderCode = builderProc->procCode();

    // invoke the closure ctor
    auto resolveClosureCtorResult = closureClass->resolveCtor();
    if (resolveClosureCtorResult.isStatus())
        return resolveClosureCtorResult.getStatus();
    auto closureCtor = resolveClosureCtorResult.getResult();

    //
    std::vector<lyric_common::TypeDef> ctorTypeArguments;
    for (const auto &spec : builderTypeArguments) {
        auto resolveAssignableResult = block->resolveAssignable(spec);
        if (resolveAssignableResult.isStatus())
            return resolveAssignableResult.getStatus();
        ctorTypeArguments.push_back(resolveAssignableResult.getResult());
    }

    lyric_typing::CallsiteReifier closureReifier(closureCtor.getParameters(), closureCtor.getRest(),
        closureCtor.getTemplateUrl(), closureCtor.getTemplateParameters(), ctorTypeArguments,
        typeSystem);

    // push the proc descriptor onto the top of the stack as first positional arg
    status = block->blockCode()->loadCall(lambdaCall->getAddress());
    if (!status.isOk())
        return status;
    auto callType = state->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Call);

    TU_RETURN_IF_NOT_OK (closureReifier.reifyNextArgument(callType));

    auto invokeClosureCtorResult = closureCtor.invokeNew(builderProc->procBlock(), closureReifier);
    if (invokeClosureCtorResult.isStatus())
        return invokeClosureCtorResult.getStatus();

    // add return instruction
    TU_RETURN_IF_NOT_OK (builderCode->writeOpcode(lyric_object::Opcode::OP_RETURN));

    /*
     * step 4: invoke the lambda assembly
     */

    lyric_assembler::CallInvoker builder(builderCall);
    lyric_typing::CallsiteReifier builderReifier(builder.getParameters(), builder.getRest(),
        builder.getTemplateUrl(), builder.getTemplateParameters(), builder.getTemplateArguments(),
        typeSystem);

    // the return value of the lambda assembly is the lambda closure
    auto invokeBuilderResult = builder.invoke(block, builderReifier);
    if (invokeBuilderResult.isStatus())
        return invokeBuilderResult.getStatus();
    return invokeBuilderResult.getResult();
}
