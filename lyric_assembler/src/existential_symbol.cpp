
#include <absl/strings/match.h>

#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
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
    lyric_object::AccessType access,
    lyric_object::DeriveType derive,
    ExistentialAddress address,
    TypeHandle *existentialType,
    ExistentialSymbol *superExistential,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : BaseSymbol(address, new ExistentialSymbolPriv()),
      m_existentialUrl(existentialUrl),
      m_state(state)
{
    TU_ASSERT (m_existentialUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->access = access;
    priv->derive = derive;
    priv->existentialType = existentialType;
    priv->existentialTemplate = nullptr;
    priv->superExistential = superExistential;
    priv->existentialBlock = std::make_unique<BlockHandle>(existentialUrl, parentBlock, false);

    TU_ASSERT (priv->existentialType != nullptr);
    TU_ASSERT (priv->superExistential != nullptr);
}

lyric_assembler::ExistentialSymbol::ExistentialSymbol(
    const lyric_common::SymbolUrl &existentialUrl,
    lyric_object::AccessType access,
    lyric_object::DeriveType derive,
    ExistentialAddress address,
    TypeHandle *existentialType,
    TemplateHandle *existentialTemplate,
    ExistentialSymbol *superExistential,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : ExistentialSymbol(
        existentialUrl,
        access,
        derive,
        address,
        existentialType,
        superExistential,
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
    lyric_importer::ExistentialImport *existentialImport,
    AssemblyState *state)
    : m_existentialUrl(existentialUrl),
      m_existentialImport(existentialImport),
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
    priv->access = lyric_object::AccessType::Public;
    priv->derive = m_existentialImport->getDerive();

    auto *existentialType = m_existentialImport->getExistentialType();
    TU_ASSIGN_OR_RAISE (priv->existentialType, typeCache->importType(existentialType));

    auto *existentialTemplate = m_existentialImport->getExistentialTemplate();
    if (existentialTemplate != nullptr) {
        TU_ASSIGN_OR_RAISE (priv->existentialTemplate, typeCache->importTemplate(existentialTemplate));
    }

    auto superExistentialUrl = m_existentialImport->getSuperExistential();
    if (superExistentialUrl.isValid()) {
        TU_ASSIGN_OR_RAISE (priv->superExistential, importCache->importExistential(superExistentialUrl));
    }

    for (auto iterator = m_existentialImport->methodsBegin(); iterator != m_existentialImport->methodsEnd(); iterator++) {
        CallSymbol *callSymbol;
        TU_ASSIGN_OR_RAISE (callSymbol, importCache->importCall(iterator->second));

        BoundMethod methodBinding;
        methodBinding.methodCall = iterator->second;
        methodBinding.access = callSymbol->getAccessType();
        methodBinding.final = false;    // FIXME: this should come from the call symbol
        priv->methods[iterator->first] = methodBinding;
    }

    auto *implCache = m_state->implCache();
    for (auto iterator = m_existentialImport->implsBegin(); iterator != m_existentialImport->implsEnd(); iterator++) {
        ImplHandle *implHandle;
        TU_ASSIGN_OR_RAISE (implHandle, implCache->importImpl(iterator->second));
        auto implUrl = iterator->first.getConcreteUrl();
        priv->impls[implUrl] = implHandle;
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
lyric_assembler::ExistentialSymbol::getAssignableType() const
{
    auto *priv = getPriv();
    return priv->existentialType->getTypeDef();
}

lyric_assembler::TypeSignature
lyric_assembler::ExistentialSymbol::getTypeSignature() const
{
    auto *priv = getPriv();
    return priv->existentialType->getTypeSignature();
}

void
lyric_assembler::ExistentialSymbol::touch()
{
    if (getAddress().isValid())
        return;
    m_state->touchExistential(this);
}

lyric_object::DeriveType
lyric_assembler::ExistentialSymbol::getDeriveType() const
{
    auto *priv = getPriv();
    return priv->derive;
}

lyric_assembler::ExistentialSymbol *
lyric_assembler::ExistentialSymbol::superExistential() const
{
    auto *priv = getPriv();
    return priv->superExistential;
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

bool
lyric_assembler::ExistentialSymbol::hasMethod(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->methods.contains(name);
}

Option<lyric_assembler::BoundMethod>
lyric_assembler::ExistentialSymbol::getMethod(const std::string &name) const
{
    auto *priv = getPriv();
    if (priv->methods.contains(name))
        return Option<BoundMethod>(priv->methods.at(name));
    return Option<BoundMethod>();
}

absl::flat_hash_map<std::string,lyric_assembler::BoundMethod>::const_iterator
lyric_assembler::ExistentialSymbol::methodsBegin() const
{
    auto *priv = getPriv();
    return priv->methods.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::BoundMethod>::const_iterator
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

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::ExistentialSymbol::declareMethod(
    const std::string &name,
    const std::vector<lyric_assembler::ParameterSpec> &parameterSpec,
    const Option<lyric_assembler::ParameterSpec> &restSpec,
    const std::vector<lyric_assembler::ParameterSpec> &ctxSpec,
    const lyric_parser::Assignable &returnSpec)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare method on imported existential {}", m_existentialUrl.toString());

    auto *priv = getPriv();

    if (absl::StartsWith(name, "__"))
        return m_state->logAndContinue(AssemblerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "declaration of private member {} is not allowed", name);
    if (absl::StartsWith(name, "_"))
        return m_state->logAndContinue(AssemblerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "declaration of protected member {} is not allowed", name);

    if (priv->methods.contains(name))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "method {} already defined for existential {}", name, m_existentialUrl.toString());

    std::vector<lyric_object::Parameter> parameters;
    Option<lyric_object::Parameter> rest;
    absl::flat_hash_set<std::string> names;
    absl::flat_hash_set<std::string> labels;

    for (const auto &p : parameterSpec) {
        auto resolveParamTypeResult = priv->existentialBlock->resolveAssignable(p.type);
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
                "parameter {} already defined for method {} on existential {}",
                p.name, name, m_existentialUrl.toString());
        names.insert(p.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for method {} on existential {}",
                p.label, name, m_existentialUrl.toString());
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        parameters.push_back(param);
    }

    for (const auto &p : ctxSpec) {
        auto resolveParamTypeResult = priv->existentialBlock->resolveAssignable(p.type);
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
                "parameter {} already defined for method {} on existential {}",
                p.name, name, m_existentialUrl.toString());
        names.insert(param.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for method {} on existential {}",
                p.label, name, m_existentialUrl.toString());
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        parameters.push_back(param);
    }

    if (!restSpec.isEmpty()) {
        const auto &p = restSpec.getValue();
        auto resolveRestTypeResult = priv->existentialBlock->resolveAssignable(p.type);
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
                "parameter {} already defined for method {} on existential {}",
                p.name, name, m_existentialUrl.toString());
        names.insert(p.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for method {} on existential {}",
                p.label, name, m_existentialUrl.toString());
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        rest = Option<lyric_object::Parameter>(param);
    }

    auto resolveReturnTypeResult = priv->existentialBlock->resolveAssignable(returnSpec);
    if (resolveReturnTypeResult.isStatus())
        return resolveReturnTypeResult.getStatus();
    auto returnType = resolveReturnTypeResult.getResult();
    m_state->typeCache()->touchType(returnType);

    // build reference path to function
    auto methodPath = m_existentialUrl.getSymbolPath().getPath();
    methodPath.push_back(name);
    auto methodUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(methodPath));
    auto callIndex = m_state->numCalls();
    auto address = CallAddress::near(callIndex);

    // construct call symbol
    auto *callSymbol = new CallSymbol(methodUrl, parameters, rest, returnType, m_existentialUrl,
        lyric_object::AccessType::Public, address, lyric_object::CallMode::Normal, priv->existentialType,
        priv->existentialBlock.get(), m_state);

    auto status = m_state->appendCall(callSymbol);
    if (status.notOk()) {
        delete callSymbol;
        return status;
    }

    // add bound method
    priv->methods[name] = { methodUrl, lyric_object::AccessType::Public, false /* final */ };

    return methodUrl;
}

tempo_utils::Result<lyric_assembler::ExistentialInvoker>
lyric_assembler::ExistentialSymbol::resolveMethod(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver)
{
    auto *priv = getPriv();

    if (!priv->methods.contains(name)) {
        if (priv->superExistential == nullptr)
            return m_state->logAndContinue(AssemblerCondition::kMissingMethod,
                tempo_tracing::LogSeverity::kError,
                "missing method {}", name);
        return priv->superExistential->resolveMethod(name, receiverType, thisReceiver);
    }

    const auto &method = priv->methods.at(name);
    auto *methodSym = m_state->symbolCache()->getSymbol(method.methodCall);
    if (methodSym == nullptr)
        m_state->throwAssemblerInvariant("missing call symbol {}", method.methodCall.toString());
    if (methodSym->getSymbolType() != SymbolType::CALL)
        m_state->throwAssemblerInvariant("invalid call symbol {}", method.methodCall.toString());
    auto *callSymbol = cast_symbol_to_call(methodSym);

    if (callSymbol->isInline())
        return ExistentialInvoker(callSymbol, callSymbol->callProc());
    if (!callSymbol->isBound())
        m_state->throwAssemblerInvariant("invalid call symbol {}", callSymbol->getSymbolUrl().toString());

    return ExistentialInvoker(this, callSymbol, receiverType);
}

bool
lyric_assembler::ExistentialSymbol::hasImpl(const lyric_common::SymbolUrl &implUrl) const
{
    auto *priv = getPriv();
    return priv->impls.contains(implUrl);
}

bool
lyric_assembler::ExistentialSymbol::hasImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return false;
    return hasImpl(implType.getConcreteUrl());
}

lyric_assembler::ImplHandle *
lyric_assembler::ExistentialSymbol::getImpl(const lyric_common::SymbolUrl &implUrl) const
{
    auto *priv = getPriv();
    auto iterator = priv->impls.find(implUrl);
    if (iterator != priv->impls.cend())
        return iterator->second;
    return nullptr;
}

lyric_assembler::ImplHandle *
lyric_assembler::ExistentialSymbol::getImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return nullptr;
    return getImpl(implType.getConcreteUrl());
}

absl::flat_hash_map<lyric_common::SymbolUrl,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::ExistentialSymbol::implsBegin() const
{
    auto *priv = getPriv();
    return priv->impls.cbegin();
}

absl::flat_hash_map<lyric_common::SymbolUrl,lyric_assembler::ImplHandle *>::const_iterator
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

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::ExistentialSymbol::declareImpl(const lyric_parser::Assignable &implSpec)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare impl on imported existential {}", m_existentialUrl.toString());

    auto *priv = getPriv();

    auto resolveImplTypeResult = priv->existentialBlock->resolveAssignable(implSpec);
    if (resolveImplTypeResult.isStatus())
        return resolveImplTypeResult.getStatus();
    auto implType = resolveImplTypeResult.getResult();

    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        m_state->throwAssemblerInvariant("invalid impl type {}", implType.toString());
    auto implUrl = implType.getConcreteUrl();

    if (priv->impls.contains(implUrl))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "impl {} already defined for existential {}", implType.toString(), m_existentialUrl.toString());

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
    if (priv->existentialTemplate != nullptr) {
        TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(
            name, implTypeHandle, conceptSymbol, m_existentialUrl, priv->existentialTemplate,
            priv->existentialBlock.get()));
    } else {
        TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(
            name, implTypeHandle, conceptSymbol, m_existentialUrl, priv->existentialBlock.get()));
    }

    priv->impls[implUrl] = implHandle;

    return implType;
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
        m_state->throwAssemblerInvariant(
            "can't put sealed type on imported existential {}", m_existentialUrl.toString());

    auto *priv = getPriv();

    if (priv->derive != lyric_object::DeriveType::Sealed)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "existential {} is not sealed", m_existentialUrl.toString());
    if (sealedType.getType() != lyric_common::TypeDefType::Concrete)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "invalid derived type {} for sealed existential {}",
            sealedType.toString(), m_existentialUrl.toString());
    auto sealedUrl = sealedType.getConcreteUrl();
    if (!m_state->symbolCache()->hasSymbol(sealedUrl))
        m_state->throwAssemblerInvariant("missing symbol {}", sealedUrl.toString());
    auto *sym = m_state->symbolCache()->getSymbol(sealedType.getConcreteUrl());
    TU_ASSERT (sym != nullptr);

    TypeHandle *derivedType = nullptr;
    switch (sym->getSymbolType()) {
        case SymbolType::CLASS:
            derivedType = cast_symbol_to_class(sym)->classType();
            break;
        case SymbolType::ENUM:
            derivedType = cast_symbol_to_enum(sym)->enumType();
            break;
        case SymbolType::EXISTENTIAL:
            derivedType = cast_symbol_to_existential(sym)->existentialType();
            break;
        case SymbolType::INSTANCE:
            derivedType = cast_symbol_to_instance(sym)->instanceType();
            break;
        case SymbolType::STRUCT:
            derivedType = cast_symbol_to_struct(sym)->structType();
            break;
        default:
            break;
    }

    if (derivedType == nullptr || derivedType->getSuperType() != priv->existentialType)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "{} does not derive from sealed existential {}",
            sealedType.toString(), m_existentialUrl.toString());

    priv->sealedTypes.insert(sealedType);

    return AssemblerStatus::ok();
}
