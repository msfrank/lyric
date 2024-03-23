
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
    BlockHandle *parentBlock,
    AssemblyState *state)
    : ConceptSymbol(
        conceptUrl,
        access,
        derive,
        address,
        conceptType,
        superConcept,
        parentBlock,
        state)
{
    auto *priv = getPriv();
    priv->conceptTemplate = conceptTemplate;
    TU_ASSERT(priv->conceptTemplate != nullptr);
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

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::ConceptSymbol::declareAction(
    const std::string &name,
    const std::vector<lyric_assembler::ParameterSpec> &parameterSpec,
    const Option<lyric_assembler::ParameterSpec> &restSpec,
    const std::vector<lyric_assembler::ParameterSpec> &ctxSpec,
    const lyric_parser::Assignable &returnSpec,
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

    AbstractResolver *resolver = priv->conceptTemplate?
        (AbstractResolver *) priv->conceptTemplate : priv->conceptBlock.get();

    std::vector<lyric_object::Parameter> parameters;
    Option<lyric_object::Parameter> rest;
    absl::flat_hash_set<std::string> names;
    absl::flat_hash_set<std::string> labels;

    for (const auto &p : parameterSpec) {
        auto resolveParamTypeResult = resolver->resolveAssignable(p.type);
        if (resolveParamTypeResult.isStatus())
            return resolveParamTypeResult.getStatus();

        lyric_object::Parameter param;
        param.index = parameters.size();
        param.name = p.name;
        param.label = !p.label.empty()? p.label : p.name;
        param.placement = !p.label.empty()? lyric_object::PlacementType::Named : lyric_object::PlacementType::List;
        param.isVariable = p.binding == lyric_parser::BindingType::VARIABLE? true : false;
        param.typeDef = resolveParamTypeResult.getResult();

        if (!p.init.isEmpty()) {
            if (param.placement != lyric_object::PlacementType::Named) {
                return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
                    tempo_tracing::LogSeverity::kError,
                    "invalid initializer for positional parameter {}; only named parameters can be default-initialized",
                    p.name);
            } else {
                param.placement = lyric_object::PlacementType::Opt;
            }
        }

        if (names.contains(p.name))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "parameter {} already defined for action {} on concept {}",
                p.name, name, m_conceptUrl.toString());
        names.insert(p.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for action {} on concept {}",
                p.label, name, m_conceptUrl.toString());
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        parameters.push_back(param);
    }

    for (const auto &p : ctxSpec) {
        auto resolveParamTypeResult = resolver->resolveAssignable(p.type);
        if (resolveParamTypeResult.isStatus())
            return resolveParamTypeResult.getStatus();

        lyric_object::Parameter param;
        param.index = parameters.size();
        param.placement = lyric_object::PlacementType::Ctx;
        param.isVariable = false;
        param.typeDef = resolveParamTypeResult.getResult();

        // if ctx parameter name is not specified, then generate a unique name
        param.name = p.name.empty()? absl::StrCat("$ctx", parameters.size()) : p.name;
        param.label = param.name;

        if (names.contains(param.name))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "parameter {} already defined for action {} on concept {}",
                p.name, name, m_conceptUrl.toString());
        names.insert(param.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for action {} on concept {}",
                p.label, name, m_conceptUrl.toString());
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        parameters.push_back(param);
    }

    if (!restSpec.isEmpty()) {
        const auto &p = restSpec.getValue();
        auto resolveRestTypeResult = resolver->resolveAssignable(p.type);
        if (resolveRestTypeResult.isStatus())
            return resolveRestTypeResult.getStatus();

        lyric_object::Parameter param;
        param.index = parameters.size();
        param.name = p.name;
        param.label = param.name;
        param.placement = lyric_object::PlacementType::Rest;
        param.isVariable = p.binding == lyric_parser::BindingType::VARIABLE? true : false;
        param.typeDef = resolveRestTypeResult.getResult();

        if (names.contains(p.name))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "parameter {} already defined for action {} on concept {}",
                p.name, name, m_conceptUrl.toString());
        names.insert(p.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for action {} on concept {}",
                p.label, name, m_conceptUrl.toString());
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        rest = Option<lyric_object::Parameter>(param);
    }

    auto resolveReturnTypeResult = resolver->resolveAssignable(returnSpec);
    if (resolveReturnTypeResult.isStatus())
        return resolveReturnTypeResult.getStatus();
    auto returnType = resolveReturnTypeResult.getResult();
    m_state->typeCache()->touchType(returnType);

    // build reference path to function
    auto methodPath = m_conceptUrl.getSymbolPath().getPath();
    methodPath.push_back(name);
    auto methodUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(methodPath));
    auto actionIndex = m_state->numActions();
    auto address = ActionAddress::near(actionIndex);

    // construct action symbol
    ActionSymbol *actionSymbol;
    if (priv->conceptTemplate != nullptr) {
        actionSymbol = new ActionSymbol(methodUrl, parameters, rest, returnType,
            m_conceptUrl, address, priv->conceptTemplate, m_state);
    } else {
        actionSymbol = new ActionSymbol(methodUrl, parameters, rest, returnType,
            m_conceptUrl, address, m_state);
    }

    auto status = m_state->appendAction(actionSymbol);
    if (status.notOk()) {
        delete actionSymbol;
        return status;
    }

    // add bound method
    priv->actions[name] = { methodUrl };

    return methodUrl;
}

tempo_utils::Result<lyric_assembler::ActionInvoker>
lyric_assembler::ConceptSymbol::resolveAction(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver) const
{
    auto *priv = getPriv();

    if (!priv->actions.contains(name))
        return m_state->logAndContinue(AssemblerCondition::kMissingAction,
            tempo_tracing::LogSeverity::kError,
            "missing action {}", name);
    const auto &actionMethod = priv->actions.at(name);
    auto *methodSym = m_state->symbolCache()->getSymbol(actionMethod.methodAction);
    if (methodSym == nullptr)
        m_state->throwAssemblerInvariant("missing action symbol {}", actionMethod.methodAction.toString());
    if (methodSym->getSymbolType() != SymbolType::ACTION)
        m_state->throwAssemblerInvariant("invalid action symbol {}", actionMethod.methodAction.toString());
    auto *action = static_cast<ActionSymbol *>(methodSym);
    return ActionInvoker(action, getAddress(), receiverType);
}

bool
lyric_assembler::ConceptSymbol::hasImpl(const lyric_common::TypeDef &implType) const
{
    auto *priv = getPriv();
    return priv->impls.contains(implType);
}

lyric_assembler::ImplHandle *
lyric_assembler::ConceptSymbol::getImpl(const lyric_common::TypeDef &implType) const
{
    auto *priv = getPriv();
    if (priv->impls.contains(implType))
        return priv->impls.at(implType);
    return nullptr;
}

absl::flat_hash_map<lyric_common::TypeDef,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::ConceptSymbol::implsBegin() const
{
    auto *priv = getPriv();
    return priv->impls.cbegin();
}

absl::flat_hash_map<lyric_common::TypeDef,lyric_assembler::ImplHandle *>::const_iterator
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

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::ConceptSymbol::declareImpl(const lyric_parser::Assignable &implSpec)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare impl on imported concept {}", m_conceptUrl.toString());

    auto *priv = getPriv();

    auto resolveImplTypeResult = priv->conceptBlock->resolveAssignable(implSpec);
    if (resolveImplTypeResult.isStatus())
        return resolveImplTypeResult.getStatus();
    auto implType = resolveImplTypeResult.getResult();

    if (priv->impls.contains(implType))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "impl {} already defined for concept {}", implType.toString(), m_conceptUrl.toString());

    // touch the impl type
    auto *implTypeHandle = m_state->typeCache()->getType(implType);
    if (implTypeHandle == nullptr)
        m_state->throwAssemblerInvariant("missing type {}", implType.toString());
    TU_RETURN_IF_NOT_OK (m_state->typeCache()->touchType(implType));

    // confirm that the impl concept exists
    auto implConcept = implType.getConcreteUrl();
    if (!m_state->symbolCache()->hasSymbol(implConcept))
        m_state->throwAssemblerInvariant("missing concept symbol {}", implConcept.toString());

    // resolve the concept symbol
    auto *conceptSym = m_state->symbolCache()->getSymbol(implConcept);
    if (conceptSym->getSymbolType() != SymbolType::CONCEPT)
        m_state->throwAssemblerInvariant("invalid concept symbol {}", implConcept.toString());
    auto *conceptSymbol = cast_symbol_to_concept(conceptSym);

    conceptSymbol->touch();

    auto *implCache = m_state->implCache();

    auto name = absl::StrCat("$impl", priv->impls.size());

    ImplHandle *implHandle;
    TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(
        name, implTypeHandle, conceptSymbol, m_conceptUrl, priv->conceptBlock.get()));

    priv->impls[implType] = implHandle;

    return implType;
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
    if (!m_state->symbolCache()->hasSymbol(sealedUrl))
        m_state->throwAssemblerInvariant("missing symbol {}", sealedUrl.toString());
    auto *sym = m_state->symbolCache()->getSymbol(sealedType.getConcreteUrl());
    TU_ASSERT (sym != nullptr);

    if (sym->getSymbolType() != SymbolType::CONCEPT || cast_symbol_to_concept(sym)->superConcept() != this)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "{} does not derive from sealed concept {}", sealedType.toString(), m_conceptUrl.toString());

    priv->sealedTypes.insert(sealedType);

    return AssemblerStatus::ok();
}
