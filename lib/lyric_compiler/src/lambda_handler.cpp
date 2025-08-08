
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/compiler_utils.h>
#include <lyric_compiler/lambda_handler.h>
#include <lyric_compiler/lambda_utils.h>
#include <lyric_compiler/pack_handler.h>
#include <lyric_compiler/proc_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::LambdaHandler::LambdaHandler(
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
lyric_compiler::LambdaHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_VV << "before LambdaHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();

    // name of the lambda function is "$lambda" + the current count of locals in the enclosing proc
    auto lambdaIdentifier = absl::StrCat("$lambda", block->blockProc()->numLocals());

    // if function is generic, then parse the template parameter list
    if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        lyric_parser::ArchetypeNode *genericNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
        TU_ASSIGN_OR_RETURN (m_lambda.templateSpec, typeSystem->parseTemplate(block, genericNode->getArchetypeNode()));
    }

    // declare the function call
    TU_ASSIGN_OR_RETURN (m_lambda.callSymbol, block->declareFunction(lambdaIdentifier,
        lyric_object::AccessType::Public, m_lambda.templateSpec.templateParameters));

    auto *resolver = m_lambda.callSymbol->callResolver();
    auto *packNode = node->getChild(0);

    // parse the parameter pack
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(block, packNode->getArchetypeNode()));

    // resolve the parameter pack
    TU_ASSIGN_OR_RETURN (m_lambda.parameterPack, typeSystem->resolvePack(resolver, packSpec));

    // parse the return type
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    TU_ASSIGN_OR_RETURN (m_lambda.returnSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));

    // resolve the return type
    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, typeSystem->resolveAssignable(resolver, m_lambda.returnSpec));

    //
    TU_ASSIGN_OR_RETURN (m_lambda.procHandle,
        m_lambda.callSymbol->defineCall(m_lambda.parameterPack, returnType));

    auto pack = std::make_unique<PackHandler>(block, driver);
    ctx.appendGrouping(std::move(pack));

    auto proc = std::make_unique<LambdaProc>(&m_lambda, block, driver);
    ctx.appendChoice(std::move(proc));

    return {};
}

tempo_utils::Status
lyric_compiler::LambdaHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_VV << "after LambdaHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *fundamentalCache = driver->getFundamentalCache();
    auto *symbolCache = driver->getSymbolCache();
    auto *typeSystem = driver->getTypeSystem();
    auto *procBuilder = m_lambda.procHandle->procCode();
    auto *lambdaFragment = procBuilder->rootFragment();

    // add return instruction
    TU_RETURN_IF_NOT_OK (lambdaFragment->returnToCaller());

    // finalize the lambda call
    TU_RETURN_IF_STATUS (m_lambda.callSymbol->finalizeCall());

    // define the lambda builder
    lyric_assembler::CallSymbol *builderCall;
    TU_ASSIGN_OR_RETURN (builderCall, define_lambda_builder(
        m_lambda.callSymbol, block, fundamentalCache, symbolCache, typeSystem));

    // invoke the lambda builder
    lyric_assembler::CallableInvoker invoker;
    TU_RETURN_IF_NOT_OK (block->prepareFunction(builderCall->getName(), invoker));

    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(invoker));

    // the return value of the lambda builder is the lambda closure
    lyric_common::TypeDef resultType;
    TU_ASSIGN_OR_RETURN (resultType, invoker.invoke(block, reifier, m_fragment));

    if (m_isSideEffect) {
        if (resultType.getType() != lyric_common::TypeDefType::NoReturn) {
            TU_RETURN_IF_NOT_OK (m_fragment->popValue());
        }
    } else {
        TU_RETURN_IF_NOT_OK (driver->pushResult(resultType));
    }

    return {};
}

lyric_compiler::LambdaProc::LambdaProc(
    Lambda *lambda,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_lambda(lambda)
{
    TU_ASSERT (m_lambda != nullptr);
}

tempo_utils::Status
lyric_compiler::LambdaProc::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto handler = std::make_unique<ProcHandler>(
        m_lambda->procHandle, /* requiresResult= */ true, getBlock(), getDriver());
    ctx.setGrouping(std::move(handler));
    return {};
}

lyric_compiler::LambdaFrom::LambdaFrom(
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_isSideEffect(isSideEffect),
      m_fragment(fragment)
{
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::LambdaFrom::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    TU_LOG_VV << "decide LambdaFrom@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *fundamentalCache = driver->getFundamentalCache();
    auto *symbolCache = driver->getSymbolCache();
    auto *typeSystem = driver->getTypeSystem();

    // resolve the source function
    lyric_common::SymbolPath symbolPath;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath));
    lyric_common::SymbolUrl symbolUrl;
    TU_ASSIGN_OR_RETURN (symbolUrl, block->resolveDefinition(symbolPath));
    lyric_assembler::AbstractSymbol *sym;
    TU_ASSIGN_OR_RETURN (sym, symbolCache->getOrImportSymbol(symbolUrl));
    if (sym->getSymbolType() != lyric_assembler::SymbolType::CALL)
        return CompilerStatus::forCondition(CompilerCondition::kInvalidSymbol,
            "lambda cannot be constructed from {}", symbolUrl.toString());

    // define the lambda builder
    lyric_assembler::CallSymbol *builderCall;
    TU_ASSIGN_OR_RETURN (builderCall, define_lambda_builder(
        cast_symbol_to_call(sym), block, fundamentalCache, symbolCache, typeSystem));

    // invoke the lambda builder
    lyric_assembler::CallableInvoker invoker;
    TU_RETURN_IF_NOT_OK (block->prepareFunction(builderCall->getName(), invoker));

    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(invoker));

    // the return value of the lambda builder is the lambda closure
    lyric_common::TypeDef resultType;
    TU_ASSIGN_OR_RETURN (resultType, invoker.invoke(block, reifier, m_fragment));

    if (m_isSideEffect) {
        if (resultType.getType() != lyric_common::TypeDefType::NoReturn) {
            TU_RETURN_IF_NOT_OK (m_fragment->popValue());
        }
    } else {
        TU_RETURN_IF_NOT_OK (driver->pushResult(resultType));
    }

    return {};
}