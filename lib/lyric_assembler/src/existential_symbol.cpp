
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_callable.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/impl_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <tempo_utils/log_message.h>

lyric_assembler::ExistentialSymbol::ExistentialSymbol(
    const lyric_common::SymbolUrl &existentialUrl,
    bool isHidden,
    lyric_object::DeriveType derive,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : BaseSymbol(new ExistentialSymbolPriv()),
      m_existentialUrl(existentialUrl),
      m_state(state)
{
    TU_ASSERT (m_existentialUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->isHidden = isHidden;
    priv->derive = derive;
    priv->isDeclOnly = isDeclOnly;
    priv->existentialTemplate = nullptr;
    priv->existentialBlock = std::make_unique<BlockHandle>(existentialUrl, parentBlock);

    TU_ASSERT (priv->existentialType != nullptr);
    TU_ASSERT (priv->superExistential != nullptr);
}

lyric_assembler::ExistentialSymbol::ExistentialSymbol(
    const lyric_common::SymbolUrl &existentialUrl,
    bool isHidden,
    lyric_object::DeriveType derive,
    TemplateHandle *existentialTemplate,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : ExistentialSymbol(
        existentialUrl,
        isHidden,
        derive,
        isDeclOnly,
        parentBlock,
        state)
{
    auto *priv = getPriv();
    priv->existentialTemplate = existentialTemplate;
    TU_ASSERT(priv->existentialTemplate != nullptr);
    for (auto it = existentialTemplate->templateParametersBegin(); it != existentialTemplate->templateParametersEnd(); it++) {
        const auto &tp = *it;
        TU_RAISE_IF_STATUS (priv->existentialBlock->declareAlias(tp.name, existentialTemplate->getTemplateUrl(), tp.index));
    }
}

lyric_assembler::ExistentialSymbol::ExistentialSymbol(
    const lyric_common::SymbolUrl &existentialUrl,
    std::shared_ptr<lyric_importer::ExistentialImport> existentialImport,
    bool isCopied,
    ObjectState *state)
    : BaseSymbol(isCopied),
      m_existentialUrl(existentialUrl),
      m_existentialImport(std::move(existentialImport)),
      m_state(state)
{
    TU_ASSERT (m_existentialUrl.isValid());
    TU_ASSERT (m_existentialImport != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::ExistentialSymbolPriv *
lyric_assembler::ExistentialSymbol::load()
{
    auto *importCache = m_state->importCache();
    auto *typeCache = m_state->typeCache();

    auto priv = std::make_unique<ExistentialSymbolPriv>();

    priv->existentialBlock = std::make_unique<BlockHandle>(
        m_existentialUrl, absl::flat_hash_map<std::string, SymbolBinding>(), m_state);

    priv->isHidden = m_existentialImport->isHidden();
    priv->derive = m_existentialImport->getDerive();
    priv->isDeclOnly = m_existentialImport->isDeclOnly();

    auto typeImport = m_existentialImport->getExistentialType().lock();
    if (typeImport == nullptr)
        throw tempo_utils::StatusException(
            AssemblerStatus::forCondition(AssemblerCondition::kImportError,
            "cannot import existential {}; missing type",
            m_existentialUrl.toString()));
    TU_ASSIGN_OR_RAISE (priv->existentialType, typeCache->importType(typeImport));

    if (m_existentialImport->hasExistentialTemplate()) {
        auto templateImport = m_existentialImport->getExistentialTemplate().lock();
        if (templateImport == nullptr)
            throw tempo_utils::StatusException(
                AssemblerStatus::forCondition(AssemblerCondition::kImportError,
                "cannot import existential {}; missing template",
                m_existentialUrl.toString()));
        TU_ASSIGN_OR_RAISE (priv->existentialTemplate, typeCache->importTemplate(templateImport));
    }

    auto superExistentialUrl = m_existentialImport->getSuperExistential();
    if (superExistentialUrl.isValid()) {
        TU_ASSIGN_OR_RAISE (priv->superExistential, importCache->importExistential(superExistentialUrl));

        auto superTypeImport = m_existentialImport->getSuperType().lock();
        if (superTypeImport == nullptr)
            throw tempo_utils::StatusException(
                AssemblerStatus::forCondition(AssemblerCondition::kImportError,
                "cannot import existential {}; missing supertype",
                m_existentialUrl.toString()));
        TU_ASSIGN_OR_RAISE (priv->superType, typeCache->importType(superTypeImport));
    }

    for (auto it = m_existentialImport->methodsBegin(); it != m_existentialImport->methodsEnd(); it++) {
        CallSymbol *callSymbol;
        TU_ASSIGN_OR_RAISE (callSymbol, importCache->importCall(it->second));
        TU_RAISE_IF_NOT_OK (priv->existentialBlock->putBinding(callSymbol));
        priv->methods[it->first] = callSymbol;
    }

    auto *implCache = m_state->implCache();
    for (auto it = m_existentialImport->implsBegin(); it != m_existentialImport->implsEnd(); it++) {
        auto implImport = it->second.lock();
        if (implImport == nullptr)
            throw tempo_utils::StatusException(AssemblerStatus::forCondition(
                AssemblerCondition::kImportError, "invalid impl import"));
        ImplHandle *implHandle;
        TU_ASSIGN_OR_RAISE (implHandle, implCache->importImpl(implImport));
        auto contract = implHandle->getContract();

        auto consumerType = contract.getConsumerType();
        if (priv->impls.contains(consumerType))
            throw tempo_utils::StatusException(AssemblerStatus::forCondition(
                AssemblerCondition::kImportError, "impl {} is already imported for {}",
                    consumerType.toString(), m_existentialUrl.toString()));
        priv->impls[consumerType] = implHandle;

        auto implementationType = contract.getImplementationType();
        if (implementationType == consumerType)
            continue;
        if (priv->impls.contains(implementationType))
            throw tempo_utils::StatusException(AssemblerStatus::forCondition(
                AssemblerCondition::kImportError, "impl {} is already imported for {}",
                    implementationType.toString(), m_existentialUrl.toString()));
        priv->impls[implementationType] = implHandle;
    }

    for (auto iterator = m_existentialImport->sealedTypesBegin(); iterator != m_existentialImport->sealedTypesEnd(); iterator++) {
        priv->sealedTypes.insert(*iterator);
    }

    return priv.release();
}

lyric_object::LinkageSection
lyric_assembler::ExistentialSymbol::getLinkage() const
{
    return lyric_object::LinkageSection::Existential;
}

lyric_assembler::SymbolType
lyric_assembler::ExistentialSymbol::getSymbolType() const
{
    return SymbolType::EXISTENTIAL;
}

lyric_common::SymbolUrl
lyric_assembler::ExistentialSymbol::getSymbolUrl() const
{
    return m_existentialUrl;
}

lyric_common::TypeDef
lyric_assembler::ExistentialSymbol::getTypeDef() const
{
    auto *priv = getPriv();
    return priv->existentialType->getTypeDef();
}

lyric_assembler::BlockHandle *
lyric_assembler::ExistentialSymbol::derefBlock()
{
    auto *priv = getPriv();
    return priv->existentialBlock.get();
}

bool
lyric_assembler::ExistentialSymbol::isDeclOnly() const
{
    auto *priv = getPriv();
    return priv->isDeclOnly;
}

lyric_object::DeriveType
lyric_assembler::ExistentialSymbol::getDeriveType() const
{
    auto *priv = getPriv();
    return priv->derive;
}

lyric_assembler::TypeHandle *
lyric_assembler::ExistentialSymbol::existentialType() const
{
    auto *priv = getPriv();
    return priv->existentialType;
}

lyric_assembler::TemplateHandle *
lyric_assembler::ExistentialSymbol::existentialTemplate() const
{
    auto *priv = getPriv();
    return priv->existentialTemplate;
}

lyric_assembler::ExistentialSymbol *
lyric_assembler::ExistentialSymbol::superExistential() const
{
    auto *priv = getPriv();
    return priv->superExistential;
}

lyric_assembler::TypeHandle *
lyric_assembler::ExistentialSymbol::superType() const
{
    auto *priv = getPriv();
    return priv->superType;
}

lyric_assembler::BlockHandle *
lyric_assembler::ExistentialSymbol::existentialBlock() const
{
    auto *priv = getPriv();
    return priv->existentialBlock.get();
}

lyric_assembler::AbstractResolver *
lyric_assembler::ExistentialSymbol::existentialResolver() const
{
    auto *priv = getPriv();
    if (priv->existentialTemplate)
        return priv->existentialTemplate;
    return priv->existentialBlock.get();
}

tempo_utils::Status
lyric_assembler::ExistentialSymbol::finalizeExistential(const lyric_common::TypeDef &superExistentialType)
{
    auto *typeCache = m_state->typeCache();

    auto *priv = getPriv();

    if (priv->superType != nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "{} is already finalized", m_existentialUrl.toString());

    ExistentialSymbol *superExistential;
    TU_ASSIGN_OR_RETURN (superExistential, priv->existentialBlock->resolveExistential(superExistentialType));

    auto superDerive = superExistential->getDeriveType();
    if (superDerive == lyric_object::DeriveType::Final)
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
            "cannot derive existential {} from {}; base existential is marked final",
            m_existentialUrl.getSymbolPath().toString(), superExistential->getSymbolUrl().toString());
    if (superDerive == lyric_object::DeriveType::Sealed && superExistential->isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
            "cannot derive existential {} from {}; sealed base existential must be located in the same module",
            m_existentialUrl.getSymbolPath().toString(), superExistential->getSymbolUrl().toString());

    // create the supertype if necessary
    TypeHandle *supertypeHandle;
    if (superExistentialType.numConcreteArguments() > 0) {
        auto superUrl = superExistentialType.getConcreteUrl();
        std::vector superArguments(superExistentialType.concreteArgumentsBegin(), superExistentialType.concreteArgumentsEnd());
        TU_ASSIGN_OR_RETURN (supertypeHandle, typeCache->declareParameterizedType(superUrl, superArguments));
    } else {
        supertypeHandle = superExistential->existentialType();
    }

    // create the type
    TypeHandle *typeHandle;
    if (priv->existentialTemplate) {
        auto placeholders = priv->existentialTemplate->getPlaceholders();
        TU_ASSIGN_OR_RETURN (typeHandle, typeCache->declareSubType(m_existentialUrl, placeholders, superExistentialType));
    } else {
        TU_ASSIGN_OR_RETURN (typeHandle, typeCache->declareSubType(m_existentialUrl, {}, superExistentialType));
    }

    priv->superExistential = superExistential;
    priv->superType = supertypeHandle;
    priv->existentialType = typeHandle;
    return {};
}

bool
lyric_assembler::ExistentialSymbol::hasMethod(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->methods.contains(name);
}

lyric_assembler::CallSymbol *
lyric_assembler::ExistentialSymbol::getMethod(const std::string &name) const
{
    auto *priv = getPriv();
    auto entry = priv->methods.find(name);
    if (entry != priv->methods.cend())
        return entry->second;
    return nullptr;
}

absl::flat_hash_map<std::string,lyric_assembler::CallSymbol *>::const_iterator
lyric_assembler::ExistentialSymbol::methodsBegin() const
{
    auto *priv = getPriv();
    return priv->methods.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::CallSymbol *>::const_iterator
lyric_assembler::ExistentialSymbol::methodsEnd() const
{
    auto *priv = getPriv();
    return priv->methods.cend();
}

tu_uint32
lyric_assembler::ExistentialSymbol::numMethods() const
{
    auto *priv = getPriv();
    return static_cast<tu_uint32>(priv->methods.size());
}

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_assembler::ExistentialSymbol::declareMethod(
    const std::string &name,
    bool isHidden)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare method on imported existential {}", m_existentialUrl.toString());

    auto *priv = getPriv();

    if (priv->methods.contains(name))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "method {} already defined for existential {}", name, m_existentialUrl.toString());

    // build reference path to function
    auto methodPath = m_existentialUrl.getSymbolPath().getPath();
    methodPath.push_back(name);
    auto methodUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(methodPath));

    // construct call symbol
    auto callSymbol = std::make_unique<CallSymbol>(methodUrl, m_existentialUrl, isHidden,
        lyric_object::CallMode::Normal, /* isFinal= */ true, priv->isDeclOnly,
        priv->existentialBlock.get(), m_state);

    CallSymbol *callPtr;
    TU_ASSIGN_OR_RETURN (callPtr, m_state->appendCall(std::move(callSymbol)));
    TU_RAISE_IF_NOT_OK (priv->existentialBlock->putBinding(callPtr));
    priv->methods[name] = callPtr;

    return callPtr;
}

tempo_utils::Status
lyric_assembler::ExistentialSymbol::prepareMethod(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    std::unique_ptr<AbstractCallable> &callable,
    bool thisOrInheritedReceiver)
{
    auto *priv = getPriv();

    auto entry = priv->methods.find(name);
    if (entry != priv->methods.cend()) {
        auto *callSymbol = entry->second;

        if (callSymbol->isInline()) {
            callable = std::make_unique<ExistentialCallable>(callSymbol, callSymbol->callProc());
            return {};
        }

        if (!callSymbol->isBound())
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "invalid call symbol {}", callSymbol->getSymbolUrl().toString());

        callable = std::make_unique<ExistentialCallable>(this, callSymbol);
        return {};
    }

    if (priv->superExistential == nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kMissingMethod,
            "missing method {}", name);
    return priv->superExistential->prepareMethod(name, receiverType, callable, thisOrInheritedReceiver);
}

bool
lyric_assembler::ExistentialSymbol::hasImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return false;
    auto *priv = getPriv();
    return priv->impls.contains(implType);
}

lyric_assembler::ImplHandle *
lyric_assembler::ExistentialSymbol::getImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return nullptr;
    auto *priv = getPriv();
    auto iterator = priv->impls.find(implType);
    if (iterator != priv->impls.cend())
        return iterator->second;
    return nullptr;
}

absl::flat_hash_map<lyric_common::TypeDef,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::ExistentialSymbol::implsBegin() const
{
    auto *priv = getPriv();
    return priv->impls.cbegin();
}

absl::flat_hash_map<lyric_common::TypeDef,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::ExistentialSymbol::implsEnd() const
{
    auto *priv = getPriv();
    return priv->impls.cend();
}

tu_uint32
lyric_assembler::ExistentialSymbol::numImpls() const
{
    auto *priv = getPriv();
    return priv->impls.size();
}

tempo_utils::Result<lyric_assembler::ImplHandle *>
lyric_assembler::ExistentialSymbol::declareImpl(const lyric_common::TypeDef &implType)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare impl on imported existential {}", m_existentialUrl.toString());

    auto *priv = getPriv();

    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid impl type {}", implType.toString());

    if (priv->impls.contains(implType))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "impl {} already defined for existential {}", implType.toString(), m_existentialUrl.toString());

    auto *implCache = m_state->implCache();

    ImplHandle *implHandle;
    if (priv->existentialTemplate != nullptr) {
        TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(implType, m_existentialUrl,
            priv->existentialTemplate, priv->isDeclOnly, priv->existentialBlock.get()));
    } else {
        TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(implType, m_existentialUrl,
            priv->isDeclOnly, priv->existentialBlock.get()));
    }

    priv->impls[implType] = implHandle;

    return implHandle;
}

bool
lyric_assembler::ExistentialSymbol::hasSealedType(const lyric_common::TypeDef &sealedType) const
{
    auto *priv = getPriv();
    return priv->sealedTypes.contains(sealedType);
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::ExistentialSymbol::sealedTypesBegin() const
{
    auto *priv = getPriv();
    return priv->sealedTypes.cbegin();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::ExistentialSymbol::sealedTypesEnd() const
{
    auto *priv = getPriv();
    return priv->sealedTypes.cend();
}

tempo_utils::Status
lyric_assembler::ExistentialSymbol::putSealedType(const lyric_common::TypeDef &sealedType)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't put sealed type on imported existential {}", m_existentialUrl.toString());

    auto *priv = getPriv();

    if (priv->derive != lyric_object::DeriveType::Sealed)
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "existential {} is not sealed", m_existentialUrl.toString());
    if (sealedType.getType() != lyric_common::TypeDefType::Concrete)
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "invalid derived type {} for sealed existential {}",
            sealedType.toString(), m_existentialUrl.toString());
    auto sealedUrl = sealedType.getConcreteUrl();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(sealedUrl));
    if (symbol->getSymbolType() != SymbolType::EXISTENTIAL)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid existential symbol {}", sealedUrl.toString());

    TypeHandle *derivedType = nullptr;
    switch (symbol->getSymbolType()) {
        case SymbolType::CLASS:
            derivedType = cast_symbol_to_class(symbol)->classType();
            break;
        case SymbolType::ENUM:
            derivedType = cast_symbol_to_enum(symbol)->enumType();
            break;
        case SymbolType::EXISTENTIAL:
            derivedType = cast_symbol_to_existential(symbol)->existentialType();
            break;
        case SymbolType::INSTANCE:
            derivedType = cast_symbol_to_instance(symbol)->instanceType();
            break;
        case SymbolType::STRUCT:
            derivedType = cast_symbol_to_struct(symbol)->structType();
            break;
        default:
            break;
    }

    if (derivedType == nullptr || derivedType->getSuperType() != priv->existentialType)
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "{} does not derive from sealed existential {}",
            sealedType.toString(), m_existentialUrl.toString());

    priv->sealedTypes.insert(sealedType);

    return {};
}
