
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/type_cache.h>

lyric_assembler::ImplHandle::ImplHandle(
    ImplOffset offset,
    const std::string &name,
    TypeHandle *implType,
    ConceptSymbol *implConcept,
    const lyric_common::SymbolUrl &receiverUrl,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : BaseHandle<ImplHandlePriv>(new ImplHandlePriv()),
      m_state(state)
{
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->offset = offset;
    priv->name = name;
    priv->implType = implType;
    priv->implConcept = implConcept;
    priv->receiverUrl = receiverUrl;
    priv->implBlock =std::make_unique<BlockHandle>(receiverUrl, parentBlock, false);

    TU_ASSERT (priv->offset.isValid());
    TU_ASSERT (!priv->name.empty());
    TU_ASSERT (priv->implType != nullptr);
    TU_ASSERT (priv->implConcept != nullptr);
    TU_ASSERT (priv->receiverUrl.isValid());
}

lyric_assembler::ImplHandle::ImplHandle(lyric_importer::ImplImport *implImport, AssemblyState *state)
    : m_implImport(implImport),
      m_state(state)
{
    TU_ASSERT (m_implImport != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::ImplOffset
lyric_assembler::ImplHandle::getOffset() const
{
    auto *priv = getPriv();
    return priv->offset;
}

std::string
lyric_assembler::ImplHandle::getName() const
{
    auto *priv = getPriv();
    return priv->name;
}

lyric_assembler::TypeHandle *
lyric_assembler::ImplHandle::implType() const
{
    auto *priv = getPriv();
    return priv->implType;
}

lyric_assembler::ConceptSymbol *
lyric_assembler::ImplHandle::implConcept() const
{
    auto *priv = getPriv();
    return priv->implConcept;
}

lyric_assembler::BlockHandle *
lyric_assembler::ImplHandle::implBlock() const
{
    auto *priv = getPriv();
    return priv->implBlock.get();
}

lyric_common::SymbolUrl
lyric_assembler::ImplHandle::getReceiverUrl() const
{
    auto *priv = getPriv();
    return priv->receiverUrl;
}

bool
lyric_assembler::ImplHandle::hasExtension(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->extensions.contains(name);
}

Option<lyric_assembler::ExtensionMethod>
lyric_assembler::ImplHandle::getExtension(const std::string &name) const
{
    auto *priv = getPriv();
    auto iterator = priv->extensions.find(name);
    if (iterator == priv->extensions.cend())
        return Option<lyric_assembler::ExtensionMethod>();
    return Option(iterator->second);
}

absl::flat_hash_map<std::string, lyric_assembler::ExtensionMethod>::const_iterator
lyric_assembler::ImplHandle::methodsBegin() const
{
    auto *priv = getPriv();
    return priv->extensions.cbegin();
}

absl::flat_hash_map<std::string, lyric_assembler::ExtensionMethod>::const_iterator
lyric_assembler::ImplHandle::methodsEnd() const
{
    auto *priv = getPriv();
    return priv->extensions.cend();
}

tu_uint32
lyric_assembler::ImplHandle::numExtensions() const
{
    auto *priv = getPriv();
    return priv->extensions.size();
}

tempo_utils::Result<lyric_assembler::ExtensionMethod>
lyric_assembler::ImplHandle::declareExtension(
    const std::string &name,
    const std::vector<ParameterSpec> &parameterSpec,
    const Option<ParameterSpec> &restSpec,
    const std::vector<ParameterSpec> &ctxSpec,
    const lyric_parser::Assignable &returnSpec)
{
    auto *priv = getPriv();

    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare extension on impl {} from imported receiver {}",
            priv->implType->getTypeDef().toString(), priv->receiverUrl.toString());

    if (priv->extensions.contains(name))
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "extension {} already defined on impl {} in receiver {}",
            name, priv->implType->getTypeDef().toString(), priv->receiverUrl.toString());

    auto actionOption = priv->implConcept->getAction(name);
    if (actionOption.isEmpty())
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "no such action {} for concept {}",
            name, priv->implConcept->getSymbolUrl().toString());

    auto &extension = priv->extensions[name];
    extension.methodAction = actionOption.getValue().methodAction;

    std::vector<lyric_object::Parameter> parameters;
    Option<lyric_object::Parameter> rest;
    absl::flat_hash_set<std::string> names;
    absl::flat_hash_set<std::string> labels;

    for (const auto &p : parameterSpec) {
        auto resolveParamTypeResult = priv->implBlock->resolveAssignable(p.type);
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
                "parameter {} already defined for extension {}", p.name, name);
        names.insert(p.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for extension {}", p.label, name);
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        parameters.push_back(param);
    }

    for (const auto &p : ctxSpec) {
        auto resolveParamTypeResult = priv->implBlock->resolveAssignable(p.type);
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
                "parameter {} already defined for extension {}", p.name, name);
        names.insert(param.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for extension {}", p.label, name);
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        parameters.push_back(param);
    }

    if (!restSpec.isEmpty()) {
        const auto &p = restSpec.getValue();
        auto resolveRestTypeResult = priv->implBlock->resolveAssignable(p.type);
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
                "parameter {} already defined for extension {}", p.name, name);
        names.insert(p.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for extension {}", p.label, name);
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        rest = Option<lyric_object::Parameter>(param);
    }

    auto resolveReturnTypeResult = priv->implBlock->resolveAssignable(returnSpec);
    if (resolveReturnTypeResult.isStatus())
        return resolveReturnTypeResult.getStatus();
    auto returnType = resolveReturnTypeResult.getResult();
    m_state->typeCache()->touchType(returnType);

    // build reference path to function
    auto methodPath = priv->receiverUrl.getSymbolPath().getPath();
    methodPath.push_back(absl::StrCat(priv->name, "$", name));
    auto methodUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(methodPath));
    auto callIndex = m_state->numCalls();
    auto address = CallAddress::near(callIndex);

    // create the type
    TypeHandle *typeHandle;
    TU_ASSIGN_OR_RETURN (typeHandle, m_state->typeCache()->declareFunctionType(returnType, parameters, rest));

    // construct call symbol
    auto *callSymbol = new CallSymbol(methodUrl, parameters, rest, returnType, priv->receiverUrl,
        lyric_object::AccessType::Public, address, lyric_object::CallMode::Normal, typeHandle,
        priv->implBlock.get(), m_state);

    auto status = m_state->appendCall(callSymbol);
    if (status.notOk()) {
        delete callSymbol;
        return status;
    }

    // update extension method call field
    extension.methodCall = methodUrl;

    return extension;
}

lyric_assembler::ImplHandlePriv *
lyric_assembler::ImplHandle::load()
{
    auto *importCache = m_state->importCache();
    auto *typeCache = m_state->typeCache();

    auto priv = std::make_unique<ImplHandlePriv>();

    auto *implType = m_implImport->getImplType();
    TU_ASSIGN_OR_RAISE (priv->implType, typeCache->importType(implType));
    TU_ASSIGN_OR_RAISE (priv->implConcept, importCache->importConcept(m_implImport->getImplConcept()));
    priv->receiverUrl = m_implImport->getReceiverUrl();

    for (auto iterator = m_implImport->extensionsBegin(); iterator != m_implImport->extensionsEnd(); iterator++) {
        auto &extension = iterator->second;

        TU_RAISE_IF_STATUS(importCache->importAction(extension.actionUrl));
        TU_RAISE_IF_STATUS(importCache->importCall(extension.callUrl));

        ExtensionMethod method;
        method.methodAction = extension.actionUrl;
        method.methodCall = extension.callUrl;
        priv->extensions[iterator->first] = method;
    }

    return priv.release();
}
