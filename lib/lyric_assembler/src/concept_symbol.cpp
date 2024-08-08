
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <lyric_assembler/abstract_resolver.h>
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/impl_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_importer/concept_import.h>

lyric_assembler::ConceptSymbol::ConceptSymbol(
    const lyric_common::SymbolUrl &conceptUrl,
    lyric_object::AccessType access,
    lyric_object::DeriveType derive,
    ConceptAddress address,
    TypeHandle *conceptType,
    ConceptSymbol *superConcept,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : BaseSymbol(address, new ConceptSymbolPriv()),
      m_conceptUrl(conceptUrl),
      m_state(state)
{
    TU_ASSERT (m_conceptUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->access = access;
    priv->derive = derive;
    priv->isDeclOnly = isDeclOnly;
    priv->conceptType = conceptType;
    priv->superConcept = superConcept;
    priv->conceptTemplate = nullptr;
    priv->conceptBlock = std::make_unique<BlockHandle>(conceptUrl, parentBlock, false);

    TU_ASSERT (priv->conceptType != nullptr);
    TU_ASSERT (priv->superConcept != nullptr);
}

lyric_assembler::ConceptSymbol::ConceptSymbol(
    const lyric_common::SymbolUrl &conceptUrl,
    lyric_object::AccessType access,
    lyric_object::DeriveType derive,
    ConceptAddress address,
    TypeHandle *conceptType,
    TemplateHandle *conceptTemplate,
    ConceptSymbol *superConcept,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : ConceptSymbol(
        conceptUrl,
        access,
        derive,
        address,
        conceptType,
        superConcept,
        isDeclOnly,
        parentBlock,
        state)
{
    auto *priv = getPriv();
    priv->conceptTemplate = conceptTemplate;
    TU_ASSERT(priv->conceptTemplate != nullptr);
    for (auto it = conceptTemplate->templateParametersBegin(); it != conceptTemplate->templateParametersEnd(); it++) {
        const auto &tp = *it;
        TU_RAISE_IF_STATUS (priv->conceptBlock->declareAlias(tp.name, conceptTemplate->getTemplateUrl(), tp.index));
    }
}

lyric_assembler::ConceptSymbol::ConceptSymbol(
    const lyric_common::SymbolUrl &conceptUrl,
    lyric_importer::ConceptImport *conceptImport,
    AssemblyState *state)
    : m_conceptUrl(conceptUrl),
      m_conceptImport(conceptImport),
      m_state(state)
{
    TU_ASSERT (m_conceptUrl.isValid());
    TU_ASSERT (m_conceptImport != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::ConceptSymbolPriv *
lyric_assembler::ConceptSymbol::load()
{
    auto *importCache = m_state->importCache();
    auto *typeCache = m_state->typeCache();

    auto priv = std::make_unique<ConceptSymbolPriv>();

    priv->isDeclOnly = m_conceptImport->isDeclOnly();
    priv->access = lyric_object::AccessType::Public;
    priv->derive = m_conceptImport->getDerive();

    auto *conceptType = m_conceptImport->getConceptType();
    TU_ASSIGN_OR_RAISE (priv->conceptType, typeCache->importType(conceptType));

    auto *conceptTemplate = m_conceptImport->getConceptTemplate();
    if (conceptTemplate != nullptr) {
        TU_ASSIGN_OR_RAISE (priv->conceptTemplate, typeCache->importTemplate(conceptTemplate));
    }

    auto superConceptUrl = m_conceptImport->getSuperConcept();
    if (superConceptUrl.isValid()) {
        TU_ASSIGN_OR_RAISE (priv->superConcept, importCache->importConcept(superConceptUrl));
    }

    for (auto iterator = m_conceptImport->actionsBegin(); iterator != m_conceptImport->actionsEnd(); iterator++) {
        TU_RAISE_IF_STATUS(importCache->importAction(iterator->second));

        ActionMethod methodBinding;
        methodBinding.methodAction = iterator->second;
        priv->actions[iterator->first] = methodBinding;
    }

    auto *implCache = m_state->implCache();
    for (auto iterator = m_conceptImport->implsBegin(); iterator != m_conceptImport->implsEnd(); iterator++) {
        ImplHandle *implHandle;
        TU_ASSIGN_OR_RAISE (implHandle, implCache->importImpl(iterator->second));
        auto implUrl = iterator->first.getConcreteUrl();
        priv->impls[implUrl] = implHandle;
    }

    for (auto iterator = m_conceptImport->sealedTypesBegin(); iterator != m_conceptImport->sealedTypesEnd(); iterator++) {
        priv->sealedTypes.insert(*iterator);
    }

    priv->conceptBlock = std::make_unique<BlockHandle>(
        m_conceptUrl, absl::flat_hash_map<std::string, SymbolBinding>(), m_state);

    return priv.release();
}

lyric_object::LinkageSection
lyric_assembler::ConceptSymbol::getLinkage() const
{
    return lyric_object::LinkageSection::Concept;
}

lyric_assembler::SymbolType
lyric_assembler::ConceptSymbol::getSymbolType() const
{
    return SymbolType::CONCEPT;
}

lyric_common::SymbolUrl
lyric_assembler::ConceptSymbol::getSymbolUrl() const
{
    return m_conceptUrl;
}

lyric_common::TypeDef
lyric_assembler::ConceptSymbol::getAssignableType() const
{
    auto *priv = getPriv();
    return priv->conceptType->getTypeDef();
}

lyric_assembler::TypeSignature
lyric_assembler::ConceptSymbol::getTypeSignature() const
{
    auto *priv = getPriv();
    return priv->conceptType->getTypeSignature();
}

void
lyric_assembler::ConceptSymbol::touch()
{
    if (getAddress().isValid())
        return;
    m_state->touchConcept(this);
}

bool
lyric_assembler::ConceptSymbol::isDeclOnly() const
{
    auto *priv = getPriv();
    return priv->isDeclOnly;
}

lyric_object::AccessType
lyric_assembler::ConceptSymbol::getAccessType() const
{
    auto *priv = getPriv();
    return priv->access;
}

lyric_object::DeriveType
lyric_assembler::ConceptSymbol::getDeriveType() const
{
    auto *priv = getPriv();
    return priv->derive;
}

lyric_assembler::ConceptSymbol *
lyric_assembler::ConceptSymbol::superConcept() const
{
    auto *priv = getPriv();
    return priv->superConcept;
}

lyric_assembler::TypeHandle *
lyric_assembler::ConceptSymbol::conceptType() const
{
    auto *priv = getPriv();
    return priv->conceptType;
}

lyric_assembler::TemplateHandle *
lyric_assembler::ConceptSymbol::conceptTemplate() const
{
    auto *priv = getPriv();
    return priv->conceptTemplate;
}

lyric_assembler::BlockHandle *
lyric_assembler::ConceptSymbol::conceptBlock() const
{
    auto *priv = getPriv();
    return priv->conceptBlock.get();
}

bool
lyric_assembler::ConceptSymbol::hasAction(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->actions.contains(name);
}

Option<lyric_assembler::ActionMethod>
lyric_assembler::ConceptSymbol::getAction(const std::string &name) const
{
    auto *priv = getPriv();
    if (priv->actions.contains(name))
        return Option<ActionMethod>(priv->actions.at(name));
    return Option<ActionMethod>();
}

absl::flat_hash_map<std::string,lyric_assembler::ActionMethod>::const_iterator
lyric_assembler::ConceptSymbol::actionsBegin() const
{
    auto *priv = getPriv();
    return priv->actions.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::ActionMethod>::const_iterator
lyric_assembler::ConceptSymbol::actionsEnd() const
{
    auto *priv = getPriv();
    return priv->actions.cend();
}

tu_uint32
lyric_assembler::ConceptSymbol::numActions() const
{
    auto *priv = getPriv();
    return static_cast<tu_uint32>(priv->actions.size());
}

tempo_utils::Result<lyric_assembler::ActionSymbol *>
lyric_assembler::ConceptSymbol::declareAction(
    const std::string &name,
    lyric_object::AccessType access)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare action on imported concept {}", m_conceptUrl.toString());

    auto *priv = getPriv();

    if (priv->actions.contains(name))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "action {} already defined for concept {}", name, m_conceptUrl.toString());

    // build reference path to function
    auto methodPath = m_conceptUrl.getSymbolPath().getPath();
    methodPath.push_back(name);
    auto methodUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(methodPath));
    auto actionIndex = m_state->numActions();
    auto address = ActionAddress::near(actionIndex);

    // construct action symbol
    ActionSymbol *actionSymbol;
    if (priv->conceptTemplate != nullptr) {
        actionSymbol = new ActionSymbol(methodUrl, m_conceptUrl, access, address, priv->conceptTemplate,
            priv->isDeclOnly, priv->conceptBlock.get(), m_state);
    } else {
        actionSymbol = new ActionSymbol(methodUrl, m_conceptUrl, access, address, priv->isDeclOnly,
            priv->conceptBlock.get(), m_state);
    }

    auto status = m_state->appendAction(actionSymbol);
    if (status.notOk()) {
        delete actionSymbol;
        return status;
    }

    // add bound method
    priv->actions[name] = { methodUrl };
    return actionSymbol;
}

tempo_utils::Status
lyric_assembler::ConceptSymbol::prepareAction(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    CallableInvoker &invoker,
    bool thisReceiver) const
{
    auto *priv = getPriv();

    if (!priv->actions.contains(name))
        return m_state->logAndContinue(AssemblerCondition::kMissingAction,
            tempo_tracing::LogSeverity::kError,
            "missing action {}", name);
    const auto &actionMethod = priv->actions.at(name);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(actionMethod.methodAction));
    if (symbol->getSymbolType() != SymbolType::ACTION)
        m_state->throwAssemblerInvariant("invalid action symbol {}", actionMethod.methodAction.toString());
    auto *actionSymbol = cast_symbol_to_action(symbol);
    actionSymbol->touch();

    auto callable = std::make_unique<ActionCallable>(actionSymbol, getAddress());
    return invoker.initialize(std::move(callable));
}

bool
lyric_assembler::ConceptSymbol::hasImpl(const lyric_common::SymbolUrl &implUrl) const
{
    auto *priv = getPriv();
    return priv->impls.contains(implUrl);
}

bool
lyric_assembler::ConceptSymbol::hasImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return false;
    return hasImpl(implType.getConcreteUrl());
}

lyric_assembler::ImplHandle *
lyric_assembler::ConceptSymbol::getImpl(const lyric_common::SymbolUrl &implUrl) const
{
    auto *priv = getPriv();
    auto iterator = priv->impls.find(implUrl);
    if (iterator != priv->impls.cend())
        return iterator->second;
    return nullptr;
}

lyric_assembler::ImplHandle *
lyric_assembler::ConceptSymbol::getImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return nullptr;
    return getImpl(implType.getConcreteUrl());
}

absl::flat_hash_map<lyric_common::SymbolUrl,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::ConceptSymbol::implsBegin() const
{
    auto *priv = getPriv();
    return priv->impls.cbegin();
}

absl::flat_hash_map<lyric_common::SymbolUrl,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::ConceptSymbol::implsEnd() const
{
    auto *priv = getPriv();
    return priv->impls.cend();
}

tu_uint32
lyric_assembler::ConceptSymbol::numImpls() const
{
    auto *priv = getPriv();
    return priv->impls.size();
}

tempo_utils::Result<lyric_assembler::ImplHandle *>
lyric_assembler::ConceptSymbol::declareImpl(const lyric_common::TypeDef &implType)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare impl on imported concept {}", m_conceptUrl.toString());

    auto *priv = getPriv();

    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        m_state->throwAssemblerInvariant("invalid impl type {}", implType.toString());
    auto implUrl = implType.getConcreteUrl();

    if (priv->impls.contains(implUrl))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "impl {} already defined for concept {}", implType.toString(), m_conceptUrl.toString());

    // touch the impl type
    lyric_assembler::TypeHandle *implTypeHandle;
    TU_ASSIGN_OR_RETURN (implTypeHandle, m_state->typeCache()->getOrMakeType(implType));
    implTypeHandle->touch();

    // confirm that the impl concept exists
    auto implConcept = implType.getConcreteUrl();
    if (!m_state->symbolCache()->hasSymbol(implConcept))
        m_state->throwAssemblerInvariant("missing concept symbol {}", implConcept.toString());

    // resolve the concept symbol
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(implConcept));
    if (symbol->getSymbolType() != SymbolType::CONCEPT)
        m_state->throwAssemblerInvariant("invalid concept symbol {}", implConcept.toString());
    auto *conceptSymbol = cast_symbol_to_concept(symbol);

    conceptSymbol->touch();

    auto *implCache = m_state->implCache();

    auto name = absl::StrCat("$impl", priv->impls.size());

    ImplHandle *implHandle;
    if (priv->conceptTemplate != nullptr) {
        TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(
            name, implTypeHandle, conceptSymbol, m_conceptUrl, priv->conceptTemplate, priv->conceptBlock.get()));
    } else {
        TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(
            name, implTypeHandle, conceptSymbol, m_conceptUrl, priv->conceptBlock.get()));
    }

    priv->impls[implUrl] = implHandle;

    return implHandle;
}

bool
lyric_assembler::ConceptSymbol::hasSealedType(const lyric_common::TypeDef &sealedType) const
{
    auto *priv = getPriv();
    return priv->sealedTypes.contains(sealedType);
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::ConceptSymbol::sealedTypesBegin() const
{
    auto *priv = getPriv();
    return priv->sealedTypes.cbegin();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::ConceptSymbol::sealedTypesEnd() const
{
    auto *priv = getPriv();
    return priv->sealedTypes.cend();
}

tempo_utils::Status
lyric_assembler::ConceptSymbol::putSealedType(const lyric_common::TypeDef &sealedType)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't put sealed type on imported concept {}", m_conceptUrl.toString());

    auto *priv = getPriv();

    if (priv->derive != lyric_object::DeriveType::Sealed)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "concept {} is not sealed", m_conceptUrl.toString());
    if (sealedType.getType() != lyric_common::TypeDefType::Concrete)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "invalid derived type {} for sealed concept {}", sealedType.toString(), m_conceptUrl.toString());
    auto sealedUrl = sealedType.getConcreteUrl();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(sealedUrl));
    if (symbol->getSymbolType() != SymbolType::CONCEPT)
        m_state->throwAssemblerInvariant("invalid concept symbol {}", sealedUrl.toString());

    if (cast_symbol_to_concept(symbol)->superConcept() != this)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "{} does not derive from sealed concept {}", sealedType.toString(), m_conceptUrl.toString());

    priv->sealedTypes.insert(sealedType);

    return {};
}
