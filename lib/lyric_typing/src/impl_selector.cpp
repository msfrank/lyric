
#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/extension_callable.h>
#include <lyric_assembler/impl_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_typing/impl_selector.h>
#include <lyric_typing/typing_result.h>

lyric_typing::ImplSelector::ImplSelector(const SummonReifier *reifier, lyric_assembler::BlockHandle *block)
    : m_reifier(reifier),
      m_block(block)
{
    TU_NOTNULL (m_reifier);
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

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_typing::ImplSelector::findGenericImplForArgument(
    const lyric_common::TypeDef &implType,
    const lyric_assembler::ActionSymbol *actionSymbol,
    const lyric_common::TypeDef &argumentType)
{
    auto *state = m_block->blockState();
    auto *symbolCache = state->symbolCache();
    auto *implCache = state->implCache();

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(argumentType.getConcreteUrl()));

    lyric_common::TypeDef definitionType;
    switch (symbol->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS:
        case lyric_assembler::SymbolType::CONCEPT:
        case lyric_assembler::SymbolType::EXISTENTIAL:
            definitionType = symbol->getTypeDef();
            break;
        default:
            return nullptr;
    }

    if (definitionType == argumentType)
        return nullptr;

    std::vector implArguments(implType.concreteArgumentsBegin(), implType.concreteArgumentsEnd());
    for (auto &implArg : implArguments) {
        if (implArg == argumentType) {
            implArg = definitionType;
        }
    }

    lyric_common::TypeDef genericImplType;
    TU_ASSIGN_OR_RETURN (genericImplType, lyric_common::TypeDef::forConcrete(
        implType.getConcreteUrl(), implArguments));
    if (genericImplType == implType)
        return nullptr;

    lyric_assembler::CallSymbol *callSymbol;
    TU_ASSIGN_OR_RETURN (callSymbol, implCache->getOrImportImplMethod(
        argumentType.getConcreteUrl(), genericImplType, actionSymbol, /* allowMissing= */ true));

    m_callsiteArguments = std::vector(argumentType.concreteArgumentsBegin(), argumentType.concreteArgumentsEnd());

    return callSymbol;
}

tempo_utils::Status
lyric_typing::ImplSelector::findImplInArgument(
    const lyric_common::TypeDef &implType,
    const lyric_assembler::ActionSymbol *actionSymbol,
    int index,
    std::unique_ptr<lyric_assembler::AbstractCallable> &callable)
{
    auto argumentType = m_reifier->getReifiedArgument(index);

    // if argument type is not concrete then it cannot participate in resolution
    if (argumentType.getType() != lyric_common::TypeDefType::Concrete)
        return {};

    auto *state = m_block->blockState();
    auto *implCache = state->implCache();

    lyric_assembler::CallSymbol *callSymbol;
    TU_ASSIGN_OR_RETURN (callSymbol, implCache->getOrImportImplMethod(
        argumentType.getConcreteUrl(), implType, actionSymbol, /* allowMissing= */ true));

    if (callSymbol == nullptr) {
        TU_ASSIGN_OR_RETURN (callSymbol, findGenericImplForArgument(implType, actionSymbol, argumentType));
        if (callSymbol == nullptr)
            return {};
    }

    // find the stack offset of the first argument which matches the argument type
    Option<tu_uint16> firstOption;
    TU_ASSIGN_OR_RETURN (firstOption, m_reifier->findFirstPlacement(argumentType));
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
lyric_typing::ImplSelector::select(std::unique_ptr<lyric_assembler::AbstractCallable> &callable)
{
    lyric_common::TypeDef consumerType;
    TU_ASSIGN_OR_RETURN (consumerType, m_reifier->reifySummonType());

    auto *actionSymbol = m_reifier->summonAction();

    std::unique_ptr<lyric_assembler::AbstractCallable> summonCallable;

    // step 1: check current block for an matching impl override
    TU_RETURN_IF_NOT_OK (findImplInsideProc(consumerType, actionSymbol, m_block, summonCallable));
    if (summonCallable != nullptr) {
        callable = std::move(summonCallable);
        return {};
    }

    // step 2: check the definition of each impl argument for a matching impl
    for (int i = 0; i < m_reifier->numReifiedArguments(); i++) {
        TU_RETURN_IF_NOT_OK (findImplInArgument(consumerType, actionSymbol, i, summonCallable));
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

std::vector<lyric_common::TypeDef>
lyric_typing::ImplSelector::getCallsiteArguments() const
{
    return m_callsiteArguments;
}