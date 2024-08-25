
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/internal/compiler_utils.h>
#include <lyric_compiler/operator_utils.h>
#include <lyric_typing/callsite_reifier.h>

tempo_utils::Status
lyric_compiler::compile_unary_operator(
    lyric_schema::LyricAstId operationId,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (driver != nullptr);

    auto *state = block->blockState();
    auto *fundamentalCache = state->fundamentalCache();
    auto *symbolCache = state->symbolCache();
    auto *typeSystem = driver->getTypeSystem();

    lyric_common::TypeDef operandType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    lyric_common::TypeDef operatorType;
    TU_ASSIGN_OR_RETURN (operatorType, internal::resolve_unary_operator_concept_type(
        fundamentalCache, operationId, operandType));

    // resolve operator impl
    lyric_common::SymbolUrl instanceUrl;
    TU_ASSIGN_OR_RETURN (instanceUrl, block->resolveImpl(operatorType));
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(instanceUrl));
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::INSTANCE)
        block->throwAssemblerInvariant("invalid instance symbol {}", instanceUrl.toString());
    auto *instanceSymbol = cast_symbol_to_instance(symbol);

    // resolve Operator impl
    auto *impl = instanceSymbol->getImpl(operatorType);
    if (impl == nullptr)
        return block->logAndContinue(CompilerCondition::kMissingImpl,
            tempo_tracing::LogSeverity::kError,
            "missing impl for {}", operatorType.toString());

    std::string actionName;
    TU_ASSIGN_OR_RETURN (actionName, internal::resolve_operator_action_name(operationId));

    lyric_assembler::DataReference instanceRef{instanceUrl, operatorType, lyric_assembler::ReferenceType::Value};
    lyric_assembler::CallableInvoker extensionInvoker;
    TU_RETURN_IF_NOT_OK (impl->prepareExtension(actionName, instanceRef, extensionInvoker));

    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(extensionInvoker));

    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(operandType));

    lyric_common::TypeDef resultType;
    TU_ASSIGN_OR_RETURN (resultType, extensionInvoker.invoke(block, reifier));

    return driver->pushResult(resultType);
}

tempo_utils::Status
lyric_compiler::compile_binary_operator(
    lyric_schema::LyricAstId operationId,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (driver != nullptr);

    auto *state = block->blockState();
    auto *fundamentalCache = state->fundamentalCache();
    auto *symbolCache = state->symbolCache();
    auto *typeSystem = driver->getTypeSystem();

    lyric_common::TypeDef rhsType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    lyric_common::TypeDef lhsType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    lyric_common::TypeDef operatorType;
    TU_ASSIGN_OR_RETURN (operatorType, internal::resolve_binary_operator_concept_type(
        fundamentalCache, operationId, lhsType, rhsType));

    // resolve operator impl
    lyric_common::SymbolUrl instanceUrl;
    TU_ASSIGN_OR_RETURN (instanceUrl, block->resolveImpl(operatorType));
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(instanceUrl));
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::INSTANCE)
        block->throwAssemblerInvariant("invalid instance symbol {}", instanceUrl.toString());
    auto *instanceSymbol = cast_symbol_to_instance(symbol);

    // resolve Operator impl
    auto *impl = instanceSymbol->getImpl(operatorType);
    if (impl == nullptr)
        return block->logAndContinue(CompilerCondition::kMissingImpl,
            tempo_tracing::LogSeverity::kError,
            "missing impl for {}", operatorType.toString());

    std::string actionName;
    TU_ASSIGN_OR_RETURN (actionName, internal::resolve_operator_action_name(operationId));

    lyric_assembler::DataReference instanceRef{instanceUrl, operatorType, lyric_assembler::ReferenceType::Value};
    lyric_assembler::CallableInvoker extensionInvoker;
    TU_RETURN_IF_NOT_OK (impl->prepareExtension(actionName, instanceRef, extensionInvoker));

    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(extensionInvoker));

    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(lhsType));
    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(rhsType));

    lyric_common::TypeDef resultType;
    TU_ASSIGN_OR_RETURN (resultType, extensionInvoker.invoke(block, reifier));

    return driver->pushResult(resultType);
}
