
#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/impl_cache.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_typing/impl_selector.h>
#include <lyric_typing/typing_result.h>

#include "lyric_assembler/extension_callable.h"
#include "lyric_assembler/lexical_variable.h"
#include "lyric_assembler/local_variable.h"

lyric_typing::ImplSelector::ImplSelector(lyric_assembler::BlockHandle *block)
    : m_block(block)
{
    TU_NOTNULL (m_block);
}

tempo_utils::Status
lyric_typing::ImplSelector::findImplInsideProc(
    const lyric_common::TypeDef &implType,
    const lyric_assembler::ActionSymbol *actionSymbol,
    lyric_assembler::BlockHandle *block,
    std::unique_ptr<lyric_assembler::AbstractCallable> &callable)
{
    auto implOption = m_block->getImpl(implType);
    if (!implOption.hasValue())
        return {};

    auto *state = block->blockState();
    auto *implCache = state->implCache();

    const auto &implRef = implOption.peekValue();
    lyric_assembler::CallSymbol *callSymbol;
    TU_ASSIGN_OR_RETURN (callSymbol, implCache->getOrImportImplMethod(
        implRef, actionSymbol, /* allowMissing= */ true));

    if (callSymbol->isInline()) {
        callable = std::make_unique<lyric_assembler::ExtensionCallable>(callSymbol);
    } else {
        callable = std::make_unique<lyric_assembler::ExtensionCallable>(callSymbol, implRef.usingRef);
    }
    return {};
}

tempo_utils::Status
lyric_typing::ImplSelector::findImplInArgument(
    const lyric_common::TypeDef &implType,
    const lyric_assembler::ActionSymbol *actionSymbol,
    int index,
    const SummonReifier &reifier,
    std::unique_ptr<lyric_assembler::AbstractCallable> &callable)
{
    auto argumentType = reifier.getReifiedArgument(index);

    // if argument type is not concrete then it cannot participate in resolution
    if (argumentType.getType() != lyric_common::TypeDefType::Concrete)
        return {};

    auto *state = m_block->blockState();
    auto *implCache = state->implCache();

    lyric_assembler::CallSymbol *callSymbol;
    TU_ASSIGN_OR_RETURN (callSymbol, implCache->getOrImportImplMethod(
        argumentType.getConcreteUrl(), implType, actionSymbol, /* allowMissing= */ true));

    if (callSymbol == nullptr)
        return {};

    // find the stack offset of the first argument which matches the argument type
    Option<tu_uint16> firstOption;
    TU_ASSIGN_OR_RETURN (firstOption, reifier.findFirstPlacement(argumentType));
    if (firstOption.isEmpty())
        return {};
    auto stackOffset = firstOption.getValue();

    if (callSymbol->isInline()) {
        callable = std::make_unique<lyric_assembler::ExtensionCallable>(callSymbol);
    } else {
        callable = std::make_unique<lyric_assembler::ExtensionCallable>(callSymbol, stackOffset);
    }
    return {};
}

tempo_utils::Status
lyric_typing::ImplSelector::findImplOutsideProc(
    const lyric_common::TypeDef &implType,
    const lyric_assembler::ActionSymbol *actionSymbol,
    lyric_assembler::BlockHandle *block,
    std::unique_ptr<lyric_assembler::AbstractCallable> &callable)
{
    auto *state = block->blockState();
    auto *symbolCache = state->symbolCache();
    auto *implCache = state->implCache();

    auto implOption = block->getImpl(implType);
    if (implOption.isEmpty())
        return {};
    const auto &implRef = implOption.peekValue();

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(
        implRef.usingRef.symbolUrl, /* allowMissing= */ false));

    if (symbol->getSymbolType() == lyric_assembler::SymbolType::BINDING) {
        auto *bindingSymbol = cast_symbol_to_binding(symbol);
        lyric_common::TypeDef targetType;
        TU_ASSIGN_OR_RETURN (targetType, bindingSymbol->resolveTarget({}));
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(targetType.getConcreteUrl()));
    }

    switch (symbol->getSymbolType()) {

        case lyric_assembler::SymbolType::ENUM:
        case lyric_assembler::SymbolType::INSTANCE:
        case lyric_assembler::SymbolType::STATIC:
            break;

        // currently we don't support impl variables outside the current proc
        // TODO: import variable as a lexical
        case lyric_assembler::SymbolType::ARGUMENT:
        case lyric_assembler::SymbolType::LOCAL:
        case lyric_assembler::SymbolType::LEXICAL:
            return {};

        // other symbol types are invalid
        case lyric_assembler::SymbolType::BINDING:
        case lyric_assembler::SymbolType::CLASS:
        case lyric_assembler::SymbolType::CONCEPT:
        case lyric_assembler::SymbolType::EXISTENTIAL:
        case lyric_assembler::SymbolType::STRUCT:
        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "symbol {} is not a valid impl variable", symbol->getSymbolUrl().toString());
    }

    lyric_assembler::CallSymbol *callSymbol;
    TU_ASSIGN_OR_RETURN (callSymbol, implCache->getOrImportImplMethod(
        implRef, actionSymbol, /* allowMissing= */ true));

    if (callSymbol->isInline()) {
        callable = std::make_unique<lyric_assembler::ExtensionCallable>(callSymbol);
    } else {
        callable = std::make_unique<lyric_assembler::ExtensionCallable>(callSymbol, implRef.usingRef);
    }
    return {};
}

tempo_utils::Status
lyric_typing::ImplSelector::select(
    const SummonReifier &reifier,
    std::unique_ptr<lyric_assembler::AbstractCallable> &callable)
{
    lyric_common::TypeDef consumerType;
    TU_ASSIGN_OR_RETURN (consumerType, reifier.reifySummonType());

    auto *actionSymbol = reifier.summonAction();

    std::unique_ptr<lyric_assembler::AbstractCallable> summonCallable;

    // step 1: check current block for an matching impl override
    TU_RETURN_IF_NOT_OK (findImplInsideProc(consumerType, actionSymbol, m_block, summonCallable));
    if (summonCallable != nullptr) {
        callable = std::move(summonCallable);
        return {};
    }

    // step 2: check the definition of each impl argument for a matching impl
    for (int i = 0; i < reifier.numReifiedArguments(); i++) {
        TU_RETURN_IF_NOT_OK (findImplInArgument(consumerType, actionSymbol, i, reifier, summonCallable));
        if (summonCallable != nullptr) {
            callable = std::move(summonCallable);
            return {};
        }
    }

    lyric_assembler::BlockHandle *block = m_block->blockParent();

    // step 3: check each ancestor block in the current proc for a matching impl
    for (; block != nullptr && block->blockProc() == m_block->blockProc(); block = block->blockParent()) {
        TU_RETURN_IF_NOT_OK (findImplInsideProc(consumerType, actionSymbol, block, summonCallable));
        if (summonCallable != nullptr) {
            callable = std::move(summonCallable);
            return {};
        }
    }

    // step 4: check each ancestor block outside of the current proc for a matching impl
    for (; block != nullptr; block = block->blockParent()) {
        TU_RETURN_IF_NOT_OK (findImplOutsideProc(consumerType, actionSymbol, block, summonCallable));
        if (summonCallable != nullptr) {
            callable = std::move(summonCallable);
            return {};
        }
    }

    // impl not found
    return TypingStatus::forCondition(TypingCondition::kInvalidType,
        "missing impl for {}", consumerType.toString());
}