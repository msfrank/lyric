
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
    bool isHidden,
    lyric_object::DeriveType derive,
    TypeHandle *conceptType,
    ConceptSymbol *superConcept,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : BaseSymbol(new ConceptSymbolPriv()),
      m_conceptUrl(conceptUrl),
      m_state(state)
{
    TU_ASSERT (m_conceptUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->isHidden = isHidden;
    priv->derive = derive;
    priv->isDeclOnly = isDeclOnly;
    priv->conceptType = conceptType;
    priv->superConcept = superConcept;
    priv->conceptTemplate = nullptr;
    priv->conceptBlock = std::make_unique<BlockHandle>(conceptUrl, parentBlock);

    TU_ASSERT (priv->conceptType != nullptr);
    TU_ASSERT (priv->superConcept != nullptr);
}

lyric_assembler::ConceptSymbol::ConceptSymbol(
    const lyric_common::SymbolUrl &conceptUrl,
    bool isHidden,
    lyric_object::DeriveType derive,
    TypeHandle *conceptType,
    TemplateHandle *conceptTemplate,
    ConceptSymbol *superConcept,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : ConceptSymbol(
        conceptUrl,
        isHidden,
        derive,
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
    bool isCopied,
    ObjectState *state)
    : BaseSymbol(isCopied),
      m_conceptUrl(conceptUrl),
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

    priv->conceptBlock = std::make_unique<BlockHandle>(
        m_conceptUrl, absl::flat_hash_map<std::string, SymbolBinding>(), m_state);

    priv->isDeclOnly = m_conceptImport->isDeclOnly();
    priv->isHidden = m_conceptImport->isHidden();
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

    for (auto it = m_conceptImport->actionsBegin(); it != m_conceptImport->actionsEnd(); it++) {
        ActionSymbol *actionSymbol;
        TU_ASSIGN_OR_RAISE (actionSymbol, importCache->importAction(it->second));
        TU_RAISE_IF_NOT_OK (priv->conceptBlock->putBinding(actionSymbol));

        ActionMethod methodBinding;
        methodBinding.methodAction = it->second;
        priv->actions[it->first] = methodBinding;
    }

    auto *implCache = m_state->implCache();
    for (auto it = m_conceptImport->implsBegin(); it != m_conceptImport->implsEnd(); it++) {
        ImplHandle *implHandle;
        TU_ASSIGN_OR_RAISE (implHandle, implCache->importImpl(it->second));
        auto implUrl = it->first.getConcreteUrl();
        priv->impls[implUrl] = implHandle;
    }

    for (auto it = m_conceptImport->sealedTypesBegin(); it != m_conceptImport->sealedTypesEnd(); it++) {
        priv->sealedTypes.insert(*it);
    }

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
lyric_assembler::ConceptSymbol::getTypeDef() const
{
    auto *priv = getPriv();
    return priv->conceptType->getTypeDef();
}

bool
lyric_assembler::ConceptSymbol::isDeclOnly() const
{
    auto *priv = getPriv();
    return priv->isDeclOnly;
}

bool
lyric_assembler::ConceptSymbol::isHidden() const
{
    auto *priv = getPriv();
    return priv->isHidden;
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
    bool isHidden)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare action on imported concept {}", m_conceptUrl.toString());

    auto *priv = getPriv();

    if (priv->actions.contains(name))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "action {} already defined for concept {}", name, m_conceptUrl.toString());

    // build reference path to function
    auto methodPath = m_conceptUrl.getSymbolPath().getPath();
    methodPath.push_back(name);
    auto methodUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(methodPath));

    // construct action symbol
    std::unique_ptr<ActionSymbol> actionSymbol;
    if (priv->conceptTemplate != nullptr) {
        actionSymbol = std::make_unique<ActionSymbol>(methodUrl, m_conceptUrl,
            isHidden, priv->conceptTemplate, priv->isDeclOnly, priv->conceptBlock.get(), m_state);
    } else {
        actionSymbol = std::make_unique<ActionSymbol>(methodUrl, m_conceptUrl, isHidden,
            priv->isDeclOnly, priv->conceptBlock.get(), m_state);
    }

    ActionSymbol *actionPtr;
    TU_ASSIGN_OR_RETURN (actionPtr, m_state->appendAction(std::move(actionSymbol)));
    TU_RETURN_IF_NOT_OK (priv->conceptBlock->putBinding(actionPtr));

    // add bound method
    priv->actions[name] = { methodUrl };

    return actionPtr;
}

tempo_utils::Status
lyric_assembler::ConceptSymbol::prepareAction(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    CallableInvoker &invoker,
    bool thisReceiver)
{
    auto *priv = getPriv();

    if (!priv->actions.contains(name))
        return AssemblerStatus::forCondition(AssemblerCondition::kMissingAction,
            "missing action {}", name);
    const auto &actionMethod = priv->actions.at(name);
    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(actionMethod.methodAction));
    if (symbol->getSymbolType() != SymbolType::ACTION)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid action symbol {}", actionMethod.methodAction.toString());
    auto *actionSymbol = cast_symbol_to_action(symbol);

    auto callable = std::make_unique<ActionCallable>(actionSymbol, this);
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
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare impl on imported concept {}", m_conceptUrl.toString());

    auto *priv = getPriv();

    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid impl type {}", implType.toString());
    auto implUrl = implType.getConcreteUrl();

    if (priv->impls.contains(implUrl))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "impl {} already defined for concept {}", implType.toString(), m_conceptUrl.toString());

    // touch the impl type
    TypeHandle *implTypeHandle;
    TU_ASSIGN_OR_RETURN (implTypeHandle, m_state->typeCache()->getOrMakeType(implType));
    auto implConcept = implType.getConcreteUrl();

    // resolve the concept symbol
    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(implConcept));
    if (symbol->getSymbolType() != SymbolType::CONCEPT)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid concept symbol {}", implConcept.toString());
    auto *conceptSymbol = cast_symbol_to_concept(symbol);

    auto *implCache = m_state->implCache();

    auto name = absl::StrCat("$impl", priv->impls.size());

    ImplHandle *implHandle;
    if (priv->conceptTemplate != nullptr) {
        TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(
            name, implTypeHandle, conceptSymbol, m_conceptUrl, priv->conceptTemplate, priv->isDeclOnly,
            priv->conceptBlock.get()));
    } else {
        TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(
            name, implTypeHandle, conceptSymbol, m_conceptUrl, priv->isDeclOnly, priv->conceptBlock.get()));
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
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't put sealed type on imported concept {}", m_conceptUrl.toString());

    auto *priv = getPriv();

    if (priv->derive != lyric_object::DeriveType::Sealed)
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "concept {} is not sealed", m_conceptUrl.toString());
    if (sealedType.getType() != lyric_common::TypeDefType::Concrete)
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "invalid derived type {} for sealed concept {}", sealedType.toString(), m_conceptUrl.toString());
    auto sealedUrl = sealedType.getConcreteUrl();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(sealedUrl));
    if (symbol->getSymbolType() != SymbolType::CONCEPT)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid concept symbol {}", sealedUrl.toString());

    if (cast_symbol_to_concept(symbol)->superConcept() != this)
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "{} does not derive from sealed concept {}", sealedType.toString(), m_conceptUrl.toString());

    priv->sealedTypes.insert(sealedType);

    return {};
}
