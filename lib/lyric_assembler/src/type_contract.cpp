
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/type_contract.h>

lyric_assembler::TypeContract::TypeContract(
    const lyric_common::TypeDef &consumerType,
    const lyric_common::TypeDef &implementationType,
    AbstractSymbol *providerSymbol,
    TemplateHandle *providerTemplate)
    : m_priv(std::make_shared<Priv>())
{
    m_priv->consumerType = consumerType;
    m_priv->implementationType = implementationType;
    m_priv->providerSymbol = providerSymbol;
    m_priv->providerTemplate = providerTemplate;
    TU_ASSERT (m_priv->consumerType.isValid());
    TU_ASSERT (m_priv->implementationType.isValid());
    TU_NOTNULL (m_priv->providerSymbol);
    TU_NOTNULL (m_priv->providerTemplate);
}

lyric_assembler::TypeContract::TypeContract(
    const lyric_common::TypeDef &consumerType,
    const lyric_common::TypeDef &implementationType,
    AbstractSymbol *providerSymbol)
    : m_priv(std::make_shared<Priv>())
{
    m_priv->consumerType = consumerType;
    m_priv->implementationType = implementationType;
    m_priv->providerSymbol = providerSymbol;
    m_priv->providerTemplate = nullptr;
    TU_ASSERT (m_priv->consumerType.isValid());
    TU_ASSERT (m_priv->implementationType.isValid());
    TU_NOTNULL (m_priv->providerSymbol);
}

bool
lyric_assembler::TypeContract::isValid() const
{
    return m_priv != nullptr;
}

/**
 * Returns the public type used to refer to the consumer. The consumer type omits
 * alias type parameters.
 *
 * @return
 */
lyric_common::TypeDef lyric_assembler::TypeContract::getConsumerType() const
{
    if (m_priv == nullptr)
        return {};
    return m_priv->consumerType;
}

/**
 * Returns the full type which the consumer implements. The implementation type
 * includes the alias type parameters.
 *
 * @return
 */
lyric_common::TypeDef lyric_assembler::TypeContract::getImplementationType() const
{
    if (m_priv == nullptr)
        return {};
    return m_priv->implementationType;
}

/**
 * Returns the symbol which provided the template the consumer is implementing.
 *
 * @return
 */
lyric_assembler::AbstractSymbol *
lyric_assembler::TypeContract::getProviderSymbol() const
{
    if (m_priv == nullptr)
        return nullptr;
    return m_priv->providerSymbol;
}

/**
 * Returns the template declared by the provider.
 *
 * @return
 */
lyric_assembler::TemplateHandle *
lyric_assembler::TypeContract::getProviderTemplate() const
{
    if (m_priv == nullptr)
        return nullptr;
    return m_priv->providerTemplate;
}

tempo_utils::Result<lyric_assembler::TypeContract>
lyric_assembler::TypeContract::load(
    AbstractSymbol *providerSymbol,
    const lyric_common::TypeDef &implementationType)
{
    TU_NOTNULL (providerSymbol);
    TU_ASSERT (implementationType.isValid());

    TemplateHandle *providerTemplate;
    switch (providerSymbol->getSymbolType()) {
        case SymbolType::CLASS:
            providerTemplate = cast_symbol_to_class(providerSymbol)->classTemplate();
            break;
        case SymbolType::CONCEPT:
            providerTemplate = cast_symbol_to_concept(providerSymbol)->conceptTemplate();
            break;
        case SymbolType::EXISTENTIAL:
            providerTemplate = cast_symbol_to_existential(providerSymbol)->existentialTemplate();
            break;
        case SymbolType::ENUM:
        case SymbolType::INSTANCE:
        case SymbolType::STRUCT:
            providerTemplate = nullptr;
            break;
        default:
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "invalid provider symbol for type contract");
    }

    if (providerTemplate == nullptr)
        return TypeContract(implementationType, implementationType, providerSymbol);

    auto templateParameters = providerTemplate->getTemplateParameters();
    std::vector implementationArguments(
        implementationType.concreteArgumentsBegin(), implementationType.concreteArgumentsEnd());

    if (implementationArguments.size() != templateParameters.size())
        return AssemblerStatus::forCondition(AssemblerCondition::kTypeError,
            "");

    std::vector<lyric_common::TypeDef> consumerArguments;

    for (int i = 0; i < templateParameters.size(); i++) {
        const auto &tp = templateParameters.at(i);
        if (tp.isAlias)
            continue;
        consumerArguments.push_back(implementationArguments.at(i));
    }

    lyric_common::TypeDef consumerType;
    TU_ASSIGN_OR_RETURN (consumerType, lyric_common::TypeDef::forConcrete(
        implementationType.getConcreteUrl(), consumerArguments));

    return TypeContract(consumerType, implementationType, providerSymbol, providerTemplate);
}

tempo_utils::Result<lyric_assembler::TypeContract>
lyric_assembler::TypeContract::load(
    AbstractSymbol *providerSymbol,
    TypeHandle *implementationTypeHandle)
{
    TU_NOTNULL (implementationTypeHandle);
    return load(providerSymbol, implementationTypeHandle->getTypeDef());
}
