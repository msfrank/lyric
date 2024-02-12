
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/extension_invoker.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_compiler/internal/compile_operator.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_typing/callsite_reifier.h>

static lyric_common::TypeDef
operation_type_to_concept_type(
    const lyric_assembler::AssemblyState *state,
    lyric_schema::LyricAstId operationId,
    const std::vector<lyric_common::TypeDef> &argList)
{
    lyric_common::SymbolUrl conceptUrl;

    switch (operationId) {
        case lyric_schema::LyricAstId::Add:
        case lyric_schema::LyricAstId::Sub:
        case lyric_schema::LyricAstId::Mul:
        case lyric_schema::LyricAstId::Div:
            if (argList.size() != 2)
                return lyric_common::TypeDef();
            return lyric_common::TypeDef::forConcrete(
                state->fundamentalCache()->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Arithmetic),
                {argList[0], argList[1]});

        case lyric_schema::LyricAstId::Neg:
            if (argList.size() != 1)
                return lyric_common::TypeDef();
            return lyric_common::TypeDef::forConcrete(
                state->fundamentalCache()->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Arithmetic),
                {argList[0], argList[0]});

        case lyric_schema::LyricAstId::And:
        case lyric_schema::LyricAstId::Or:
            if (argList.size() != 2)
                return lyric_common::TypeDef();
            return lyric_common::TypeDef::forConcrete(
                state->fundamentalCache()->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Proposition),
                {argList[0], argList[1]});

        case lyric_schema::LyricAstId::Not:
            if (argList.size() != 1)
                return lyric_common::TypeDef();
            return lyric_common::TypeDef::forConcrete(
                state->fundamentalCache()->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Proposition),
                {argList[0], argList[0]});

        case lyric_schema::LyricAstId::IsEq:
            if (argList.size() != 2)
                return lyric_common::TypeDef();
            return lyric_common::TypeDef::forConcrete(
                state->fundamentalCache()->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Equality),
                {argList[0], argList[1]});

        case lyric_schema::LyricAstId::IsLt:
        case lyric_schema::LyricAstId::IsLe:
        case lyric_schema::LyricAstId::IsGt:
        case lyric_schema::LyricAstId::IsGe:
            if (argList.size() != 2)
                return lyric_common::TypeDef();
            return lyric_common::TypeDef::forConcrete(
                state->fundamentalCache()->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Comparison),
                {argList[0], argList[1]});

        default:
            return lyric_common::TypeDef();
    }
}

static std::string
operation_type_to_action_name(lyric_schema::LyricAstId operationId)
{
    switch (operationId) {
        case lyric_schema::LyricAstId::Add:
            return std::string("add");
        case lyric_schema::LyricAstId::Sub:
            return std::string("subtract");
        case lyric_schema::LyricAstId::Mul:
            return std::string("multiply");
        case lyric_schema::LyricAstId::Div:
            return std::string("divide");
        case lyric_schema::LyricAstId::Neg:
            return std::string("negate");
        case lyric_schema::LyricAstId::And:
            return std::string("conjunct");
        case lyric_schema::LyricAstId::Or:
            return std::string("disjunct");
        case lyric_schema::LyricAstId::Not:
            return std::string("complement");
        case lyric_schema::LyricAstId::IsEq:
            return std::string("equals");
        case lyric_schema::LyricAstId::IsLt:
            return std::string("lessthan");
        case lyric_schema::LyricAstId::IsLe:
            return std::string("lessequals");
        case lyric_schema::LyricAstId::IsGt:
            return std::string("greaterthan");
        case lyric_schema::LyricAstId::IsGe:
            return std::string("greaterequals");
        default:
            return std::string();
    }
}

static tempo_utils::Result<lyric_common::TypeDef>
compile_is_a(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());
    auto *state = moduleEntry.getState();

    moduleEntry.checkClassAndChildCountOrThrow(walker, lyric_schema::kLyricAstIsAClass, 2);

    auto *code = block->blockCode();
    tempo_utils::Status status;

    // push lhs expression onto the stack
    auto compileExprResult = lyric_compiler::internal::compile_expression(block, walker.getChild(0), moduleEntry);
    if (compileExprResult.isStatus())
        return compileExprResult.getStatus();

    // pop expression result and push expression type descriptor onto the stack
    status = code->writeOpcode(lyric_object::Opcode::OP_TYPE_OF);
    if (!status.isOk())
        return status;

    // resolve isA type
    auto *typeSystem = moduleEntry.getTypeSystem();
    auto resolveAssignableType = typeSystem->resolveAssignable(block, walker.getChild(1));
    if (resolveAssignableType.isStatus())
        return resolveAssignableType;
    auto isAType = resolveAssignableType.getResult();
    if (!state->typeCache()->hasType(isAType))
        block->throwAssemblerInvariant("missing type {}", isAType.toString());
    state->typeCache()->touchType(isAType);

    // push isA type descriptor onto the stack
    switch (isAType.getType()) {
        case lyric_common::TypeDefType::Concrete: {
            auto *typeHandle = state->typeCache()->getType(isAType);
            status = code->loadType(typeHandle->getAddress());
            if (status.notOk())
                return status;
            break;
        }
        default:
            block->throwAssemblerInvariant("type comparison for unimplemented {}", isAType.toString());
    }


    // perform type comparison
    status = code->writeOpcode(lyric_object::Opcode::OP_TYPE_CMP);
    if (!status.isOk())
        return status;

    // if lhs type equals or extends rhs, then push true onto the stack
    auto predicateJumpResult = code->jumpIfGreaterThan();
    if (predicateJumpResult.isStatus())
        return predicateJumpResult.getStatus();
    auto predicateJump = predicateJumpResult.getResult();
    status = code->loadBool(true);
    if (!status.isOk())
        return status;

    auto consequentJumpResult = code->jump();
    if (consequentJumpResult.isStatus())
        return consequentJumpResult.getStatus();
    auto consequentJump = consequentJumpResult.getResult();

    // otherwise if lhs does not equal or extend rhs, then push false onto the stack
    auto alternativeEnterResult = code->makeLabel();
    if (alternativeEnterResult.isStatus())
        return alternativeEnterResult.getStatus();
    auto alternativeEnter = alternativeEnterResult.getResult();
    status = code->loadBool(false);
    if (!status.isOk())
        return status;

    auto alternativeExitResult = code->makeLabel();
    if (alternativeExitResult.isStatus())
        return alternativeExitResult.getStatus();
    auto alternativeExit = alternativeExitResult.getResult();

    // patch predicate jump to alternative enter label
    status = code->patch(predicateJump, alternativeEnter);
    if (!status.isOk())
        return status;

    // patch consequent jump to alternative exit label
    status = code->patch(consequentJump, alternativeExit);
    if (!status.isOk())
        return status;

    // result is always a Bool
    return block->blockState()->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::internal::compile_operator_call(
    lyric_assembler::BlockHandle *block,
    lyric_schema::LyricAstId operationId,
    const std::vector<lyric_common::TypeDef> &argList,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (!argList.empty());
    auto *typeSystem = moduleEntry.getTypeSystem();
    auto *state = moduleEntry.getState();

    auto operatorType = operation_type_to_concept_type(block->blockState(), operationId, argList);
    if (!operatorType.isValid())
        block->throwAssemblerInvariant("invalid type {}", operatorType.toString());

    // resolve Operator instance for the receiver
    auto resolveInstanceResult = block->resolveImpl(operatorType);
    if (resolveInstanceResult.isStatus())
        return resolveInstanceResult.getStatus();
    auto instanceUrl = resolveInstanceResult.getResult();
    auto *symbol = state->symbolCache()->getSymbol(instanceUrl);
    if (symbol == nullptr)
        block->throwAssemblerInvariant("missing instance symbol {}", instanceUrl.toString());
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::INSTANCE)
        block->throwAssemblerInvariant("invalid instance symbol {}", instanceUrl.toString());
    auto *instanceSymbol = cast_symbol_to_instance(symbol);

    // resolve Operator impl
    auto *impl = instanceSymbol->getImpl(operatorType);
    if (impl == nullptr)
        return block->logAndContinue(CompilerCondition::kMissingImpl,
            tempo_tracing::LogSeverity::kError,
            "missing impl for {}", operatorType.toString());

    auto extensionName = operation_type_to_action_name(operationId);
    auto extensionOption = impl->getExtension(extensionName);
    if (extensionOption.isEmpty())
        return block->logAndContinue(CompilerCondition::kMissingAction,
            tempo_tracing::LogSeverity::kError,
            "missing extension {} for impl {}", extensionName, operatorType.toString());
    auto extension = extensionOption.getValue();

    auto extensionUrl = extension.methodCall;
    symbol = block->blockState()->symbolCache()->getSymbol(extensionUrl);
    if (symbol == nullptr)
        block->throwAssemblerInvariant("missing call symbol {}", extensionUrl.toString());
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::CALL)
        block->throwAssemblerInvariant("invalid call symbol {}", extensionUrl.toString());
    auto *extensionCall = cast_symbol_to_call(symbol);

    lyric_assembler::ExtensionInvoker extensionInvoker;
    if (extensionCall->isInline()) {
        extensionInvoker = lyric_assembler::ExtensionInvoker(extensionCall, extensionCall->callProc());
    } else if (extensionCall->isBound()) {
        symbol = block->blockState()->symbolCache()->getSymbol(operatorType.getConcreteUrl());
        if (symbol == nullptr)
            block->throwAssemblerInvariant("missing concept symbol {}", operatorType.getConcreteUrl().toString());
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
            block->throwAssemblerInvariant("invalid concept symbol {}", operatorType.getConcreteUrl().toString());
        auto *conceptSymbol = cast_symbol_to_concept(symbol);

        auto resolveActionResult = conceptSymbol->getAction(extensionName);
        if (resolveActionResult.isEmpty())
            block->throwAssemblerInvariant("missing action {} for concept symbol {}",
                extensionName, operatorType.getConcreteUrl().toString());
        auto action = resolveActionResult.getValue();
        symbol = block->blockState()->symbolCache()->getSymbol(action.methodAction);
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::ACTION)
            block->throwAssemblerInvariant("invalid action symbol {}", action.methodAction.toString());
        auto *actionSymbol = cast_symbol_to_action(symbol);

        lyric_assembler::SymbolBinding binding = { instanceUrl, operatorType, lyric_parser::BindingType::VALUE };
        instanceSymbol->touch();

        extensionInvoker = lyric_assembler::ExtensionInvoker(conceptSymbol, actionSymbol, operatorType, binding);
    } else {
        block->throwAssemblerInvariant("invalid extension call {}", extensionUrl.toString());
    }

    lyric_typing::CallsiteReifier reifier(extensionInvoker.getParameters(), extensionInvoker.getRest(),
        extensionInvoker.getTemplateUrl(), extensionInvoker.getTemplateParameters(),
        extensionInvoker.getTemplateArguments(), typeSystem);
    for (const auto &arg : argList) {
        auto status = reifier.reifyNextArgument(arg);
        if (!status.isOk())
            return status;
    }

    auto invokeExtensionResult = extensionInvoker.invoke(block, reifier);
    if (invokeExtensionResult.isStatus())
        return invokeExtensionResult.getStatus();
    return invokeExtensionResult.getResult();
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::internal::compile_operator(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    lyric_schema::LyricAstId operationId{};
    moduleEntry.parseIdOrThrow(walker, lyric_schema::kLyricAstVocabulary, operationId);

    if (operationId == lyric_schema::LyricAstId::IsA)
        return compile_is_a(block, walker, moduleEntry);

    std::vector<lyric_common::TypeDef> argList;
    for (int i = 0; i < walker.numChildren(); i++) {
        auto compileExprResult = compile_expression(block, walker.getChild(i), moduleEntry);
        if (compileExprResult.isStatus())
            return compileExprResult.getStatus();
        argList.push_back(compileExprResult.getResult());
    }

    return compile_operator_call(block, operationId, argList, moduleEntry);
}
