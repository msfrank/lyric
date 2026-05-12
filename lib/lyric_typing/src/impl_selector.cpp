
#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_typing/impl_selector.h>
#include <lyric_typing/typing_result.h>

lyric_typing::ImplSelector::ImplSelector(lyric_assembler::BlockHandle *block)
    : m_block(block)
{
    TU_NOTNULL (m_block);
}

tempo_utils::Result<Option<lyric_assembler::ImplReference>>
lyric_typing::ImplSelector::findImplInArgument(
    const lyric_common::TypeDef &implType,
    const SummonReifier &reifier,
    int index,
    lyric_assembler::CodeFragment *fragment)
{
    auto argumentType = reifier.getReifiedArgument(index);

    auto *state = m_block->blockState();
    auto *symbolCache = state->symbolCache();

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(argumentType.getConcreteUrl()));

    lyric_assembler::ImplHandle *implHandle;

    switch (symbol->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = lyric_assembler::cast_symbol_to_class(symbol);
            implHandle = classSymbol->getImpl(implType);
            break;
        }
        case lyric_assembler::SymbolType::CONCEPT: {
            auto *conceptSymbol = lyric_assembler::cast_symbol_to_concept(symbol);
            implHandle = conceptSymbol->getImpl(implType);
            break;
        }
        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = lyric_assembler::cast_symbol_to_enum(symbol);
            implHandle = enumSymbol->getImpl(implType);
            break;
        }
        case lyric_assembler::SymbolType::EXISTENTIAL: {
            auto *existentialSymbol = lyric_assembler::cast_symbol_to_existential(symbol);
            implHandle = existentialSymbol->getImpl(implType);
            break;
        }
        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = lyric_assembler::cast_symbol_to_instance(symbol);
            implHandle = instanceSymbol->getImpl(implType);
            break;
        }
        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = lyric_assembler::cast_symbol_to_struct(symbol);
            implHandle = structSymbol->getImpl(implType);
            break;
        }
        default:
            return TypingStatus::forCondition(TypingCondition::kInvalidType,
                "invalid definition {} for impl {}", symbol->getSymbolUrl().toString(), implType.toString());
    }

    if (implHandle == nullptr)
        return Option<lyric_assembler::ImplReference>();
    auto contract = implHandle->getContract();

    // find the stack offset of the first argument which matches the argument type
    Option<tu_uint16> firstOption;
    TU_ASSIGN_OR_RETURN (firstOption, reifier.findFirstPlacement(argumentType));
    if (firstOption.isEmpty())
        return Option<lyric_assembler::ImplReference>();
    auto stackOffset = firstOption.getValue();

    // declare a temporary to hold the impl receiver
    lyric_assembler::DataReference ref;
    TU_ASSIGN_OR_RETURN (ref, m_block->declareTemporary(argumentType, false));

    // push the impl receiver argument into the top of the stack
    TU_RETURN_IF_NOT_OK (fragment->rpickValue(stackOffset));

    // store the impl receiver in the temporary
    TU_RETURN_IF_NOT_OK (fragment->storeRef(ref, true));

    lyric_assembler::ImplReference implRef;
    implRef.usingRef = ref;
    implRef.implType = contract.getImplementationType();
    return Option(implRef);
}

tempo_utils::Result<Option<lyric_assembler::ImplReference>>
lyric_typing::ImplSelector::findImplOutsideProc(
    const lyric_common::TypeDef &implType,
    lyric_assembler::BlockHandle *block)
{
    auto *state = block->blockState();
    auto *symbolCache = state->symbolCache();

    auto implOption = block->getImpl(implType);
    if (implOption.isEmpty())
        return Option<lyric_assembler::ImplReference>();
    const auto &implRef = implOption.peekValue();

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(implRef.usingRef.symbolUrl));

    if (symbol->getSymbolType() == lyric_assembler::SymbolType::BINDING) {
        auto *bindingSymbol = cast_symbol_to_binding(symbol);
        lyric_common::TypeDef targetType;
        TU_ASSIGN_OR_RETURN (targetType, bindingSymbol->resolveTarget({}));
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(targetType.getConcreteUrl()));
    }

    switch (symbol->getSymbolType()) {

        case lyric_assembler::SymbolType::ENUM:
        case lyric_assembler::SymbolType::INSTANCE:
            return implOption;

        // currently we don't support impl variables outside the current proc
        // TODO: import variable as a lexical
        case lyric_assembler::SymbolType::ARGUMENT:
        case lyric_assembler::SymbolType::LOCAL:
        case lyric_assembler::SymbolType::LEXICAL:
            return Option<lyric_assembler::ImplReference>();

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
}

tempo_utils::Result<lyric_assembler::ImplReference>
lyric_typing::ImplSelector::select(
    const SummonReifier &reifier,
    lyric_assembler::CodeFragment *fragment)
{
    lyric_common::TypeDef consumerType;
    TU_ASSIGN_OR_RETURN (consumerType, reifier.reifySummonType());

    Option<lyric_assembler::ImplReference> implOption;

    // step 1: check current block for an matching impl override
    implOption = m_block->getImpl(consumerType);
    if (implOption.hasValue())
        return implOption.getValue();

    // step 2: check the definition of each impl argument for a matching impl
    for (int i = 0; i < reifier.numReifiedArguments(); i++) {
        TU_ASSIGN_OR_RETURN (implOption, findImplInArgument(consumerType, reifier, i, fragment));
        if (implOption.hasValue())
            return implOption.getValue();
    }

    lyric_assembler::BlockHandle *block = m_block->blockParent();

    // step 3: check each ancestor block in the current proc for a matching impl
    for (; block != nullptr && block->blockProc() == m_block->blockProc(); block = block->blockParent()) {
        implOption = block->getImpl(consumerType);
        if (implOption.hasValue())
            return implOption.getValue();
    }

    // step 4: check each ancestor block outside of the current proc for a matching impl
    for (; block != nullptr; block = block->blockParent()) {
        TU_ASSIGN_OR_RETURN (implOption, findImplOutsideProc(consumerType, block));
        if (implOption.hasValue())
            return implOption.getValue();
    }

    // impl not found
    return TypingStatus::forCondition(TypingCondition::kInvalidType,
        "missing impl for {}", consumerType.toString());
}

// tempo_utils::Status
// lyric_typing::ImplSelector::initialize(lyric_assembler::ActionSymbol *actionSymbol)
// {
//     TU_NOTNULL (actionSymbol);
//
//     if (m_initialized)
//         return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
//             "action invoker is already initialized");
//
//     m_actionSymbol = actionSymbol;
//     m_conceptUrl = actionSymbol->getReceiverUrl();
//
//     auto *templateHandle = actionSymbol->actionTemplate();
//     if (templateHandle != nullptr) {
//         m_templateParameters = std::vector(
//             templateHandle->templateParametersBegin(), templateHandle->templateParametersEnd());
//         for (const auto &tp : m_templateParameters) {
//             if (!tp.isAlias) {
//                 m_implTemplateParametersIndex.push_back(tp.index);
//             }
//         }
//         m_contractArgumentTypes.resize(m_templateParameters.size());
//     }
//
//     m_unifiedParameters.insert(m_unifiedParameters.cbegin(),
//         actionSymbol->listPlacementBegin(), actionSymbol->listPlacementEnd());
//     m_unifiedParameters.insert(m_unifiedParameters.cend(),
//         actionSymbol->namedPlacementBegin(), actionSymbol->namedPlacementEnd());
//     auto *restPlacement = actionSymbol->restPlacement();
//     if (restPlacement != nullptr) {
//         m_unifiedParameters.push_back(*restPlacement);
//     }
//
//     m_initialized = true;
//     return {};
// }
//
// tempo_utils::Status
// lyric_typing::ImplSelector::reifyAliasArgument(const lyric_assembler::BindingSymbol *bindingSymbol)
// {
//     TU_NOTNULL (bindingSymbol);
//     auto *companionTypeHandle = bindingSymbol->companionType();
//
//     // we ignore alias if it does not have a companion type
//     if (companionTypeHandle == nullptr)
//         return {};
//     auto companionType = companionTypeHandle->getTypeDef();
//
//     if (companionType.getType() != lyric_common::TypeDefType::Placeholder)
//         return TypingStatus::forCondition(
//             TypingCondition::kTypingInvariant, "invalid companion type for alias");
//
//     // we ignore alias if its companion type does not match the concept
//     auto templateUrl = companionType.getPlaceholderTemplateUrl();
//     if (templateUrl != m_conceptUrl)
//         return {};
//
//     auto index = companionType.getPlaceholderIndex();
//     TU_ASSERT (index < m_templateParameters.size());
//
//     if (m_contractArgumentTypes.at(index).isValid())
//         return TypingStatus::forCondition(
//             TypingCondition::kTypingInvariant, "alias type argument is already declared");
//
//     auto *targetTypeHandle = bindingSymbol->targetType();
//     if (targetTypeHandle == nullptr)
//         return TypingStatus::forCondition(
//             TypingCondition::kTypingInvariant, "invalid target type for alias");
//     auto argumentType = targetTypeHandle->getTypeDef();
//
//     auto *state = m_block->blockState();
//     const auto &tp = m_templateParameters.at(index);
//     TU_RETURN_IF_NOT_OK (internal::check_placeholder(tp, argumentType, state));
//
//     m_contractArgumentTypes[tp.index] = argumentType;
//
//     return {};
// }
//
// tempo_utils::Result<lyric_common::TypeDef>
// lyric_typing::ImplSelector::invoke(lyric_assembler::CodeFragment *fragment)
// {
//     // verify all impl argument types have been reified
//     if (m_implArgumentTypes.size() != m_implTemplateParametersIndex.size())
//         return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
//             "wrong number of impl type arguments; expected {} but found {}",
//             m_implTemplateParametersIndex.size(), m_implArgumentTypes.size());
//
//     // resolve the impl
//     lyric_assembler::ImplHandle *implHandle;
//     TU_ASSIGN_OR_RETURN (implHandle, resolveImplHandle());
//
//     lyric_common::TypeDef implementationType;
//     TU_ASSIGN_OR_RETURN (implementationType, lyric_common::TypeDef::forConcrete(m_conceptUrl, m_contractArgumentTypes));
//
//     lyric_assembler::CallableInvoker invoker;
//     TU_RETURN_IF_NOT_OK (implHandle->prepareExtension(actionName, implRef.usingRef, invoker));
//
//     invoker.initialize()
// }
