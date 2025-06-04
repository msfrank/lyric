
#include <lyric_assembler/argument_variable.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/internal/import_proc.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/synthetic_symbol.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_assembler/type_set.h>

#include "lyric_assembler/local_variable.h"

/**
 * @brief Construct a call symbol for the $entry call.
 */
lyric_assembler::CallSymbol::CallSymbol(
    const lyric_common::SymbolUrl &entryUrl,
    BlockHandle *rootBlock,
    ObjectState *state)
    : BaseSymbol(new CallSymbolPriv()),
      m_callUrl(entryUrl),
      m_state(state)
{
    TU_ASSERT (m_callUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->returnType = lyric_common::TypeDef::noReturn();
    priv->access = lyric_object::AccessType::Public;
    priv->mode = lyric_object::CallMode::Normal;
    priv->isNoReturn =  true;
    priv->callType = nullptr;
    priv->callTemplate = nullptr;
    priv->parentBlock = nullptr;
    priv->proc = std::make_unique<ProcHandle>(entryUrl, rootBlock, m_state);
}

/**
 * @brief Construct a CallSymbol for a generic bound method.
 * @param callUrl The fully qualified symbol url.
 * @param parameters A vector of positional, named, and context call parameters.
 * @param rest The rest parameter, or an empty Option<Parameter> if there is no
 *   rest parameter.
 * @param returnType The return type of the call.
 * @param receiverUrl The class or instance receiver bound to the method, or empty
 *   SymbolUrl if there is no receiver.
 * @param access
 * @param callType
 * @param callTemplate
 * @param parentBlock
 * @param state
 */
lyric_assembler::CallSymbol::CallSymbol(
    const lyric_common::SymbolUrl &callUrl,
    const lyric_common::SymbolUrl &receiverUrl,
    lyric_object::AccessType access,
    lyric_object::CallMode mode,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : BaseSymbol(new CallSymbolPriv()),
      m_callUrl(callUrl),
      m_state(state)
{
    TU_ASSERT (m_callUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->receiverUrl = receiverUrl;
    priv->access = access;
    priv->isDeclOnly = isDeclOnly;
    priv->mode = mode;
    priv->callType = nullptr;
    priv->callTemplate = nullptr;
    priv->parentBlock = parentBlock;

    TU_ASSERT (priv->receiverUrl.isValid());
    TU_ASSERT (priv->access != lyric_object::AccessType::Invalid);
    TU_ASSERT (priv->mode != lyric_object::CallMode::Invalid);
    TU_ASSERT (priv->parentBlock != nullptr);
}

/**
 * @brief Construct a CallSymbol for a bound method.
 *
 */
lyric_assembler::CallSymbol::CallSymbol(
    const lyric_common::SymbolUrl &callUrl,
    const lyric_common::SymbolUrl &receiverUrl,
    lyric_object::AccessType access,
    lyric_object::CallMode mode,
    TemplateHandle *callTemplate,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : CallSymbol(
        callUrl,
        receiverUrl,
        access,
        mode,
        isDeclOnly,
        parentBlock,
        state)
{
    auto *priv = getPriv();
    priv->callTemplate = callTemplate;
    TU_ASSERT (priv->callTemplate != nullptr);
}

/**
 * @brief Construct a CallSymbol for a generic free function.
 * @param callUrl The fully qualified symbol url.
 * @param parameters A vector of positional, named, and context call parameters.
 * @param rest The rest parameter, or an empty Option<Parameter> if there is no
 *   rest parameter.
 * @param returnType The return type of the call.
 * @param access
 * @param callType
 * @param callTemplate
 * @param parentBlock
 * @param state
 */
lyric_assembler::CallSymbol::CallSymbol(
    const lyric_common::SymbolUrl &callUrl,
    lyric_object::AccessType access,
    lyric_object::CallMode mode,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : BaseSymbol(new CallSymbolPriv()),
      m_callUrl(callUrl),
      m_state(state)
{
    TU_ASSERT (m_callUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->access = access;
    priv->isDeclOnly = isDeclOnly;
    priv->mode = mode;
    priv->callType = nullptr;
    priv->callTemplate = nullptr;
    priv->parentBlock = parentBlock;

    TU_ASSERT (priv->access != lyric_object::AccessType::Invalid);
    TU_ASSERT (priv->mode != lyric_object::CallMode::Invalid);
    TU_ASSERT (priv->parentBlock != nullptr);
}

/**
 * @brief Construct a CallSymbol for a free function.
 *
 */
lyric_assembler::CallSymbol::CallSymbol(
    const lyric_common::SymbolUrl &callUrl,
    lyric_object::AccessType access,
    lyric_object::CallMode mode,
    TemplateHandle *callTemplate,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : CallSymbol(
        callUrl,
        access,
        mode,
        isDeclOnly,
        parentBlock,
        state)
{
    auto *priv = getPriv();
    priv->callTemplate = callTemplate;
    TU_ASSERT (priv->callTemplate != nullptr);
}

/**
 * @brief Construct a CallSymbol for an imported call.
 *
 */
lyric_assembler::CallSymbol::CallSymbol(
    const lyric_common::SymbolUrl &callUrl,
    lyric_importer::CallImport *callImport,
    bool isCopied,
    ObjectState *state)
    : BaseSymbol(isCopied),
      m_callUrl(callUrl),
      m_callImport(callImport),
      m_state(state)
{
    TU_ASSERT (m_callUrl.isValid());
    TU_ASSERT (m_callImport != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::CallSymbolPriv *
lyric_assembler::CallSymbol::load()
{
    auto *typeCache = m_state->typeCache();
    auto *options = m_state->getOptions();

    auto priv = std::make_unique<CallSymbolPriv>();

    priv->isDeclOnly = m_callImport->isDeclOnly();

    for (auto it = m_callImport->listParametersBegin(); it != m_callImport->listParametersEnd(); it++) {
        Parameter p;
        p.index = it->index;
        p.name = it->name;
        p.placement = it->placement;
        p.isVariable = it->isVariable;

        TypeHandle *paramType = nullptr;
        TU_ASSIGN_OR_RAISE (paramType, typeCache->importType(it->type));
        p.typeDef = paramType->getTypeDef();

        priv->listParameters.push_back(p);
        priv->parametersMap[p.name] = p;
    }

    for (auto it = m_callImport->namedParametersBegin(); it != m_callImport->namedParametersEnd(); it++) {
        Parameter p;
        p.index = it->index;
        p.name = it->name;
        p.placement = it->placement;
        p.isVariable = it->isVariable;

        TypeHandle *paramType = nullptr;
        TU_ASSIGN_OR_RAISE (paramType, typeCache->importType(it->type));
        p.typeDef = paramType->getTypeDef();

        priv->namedParameters.push_back(p);
        priv->parametersMap[p.name] = p;
    }

    if (m_callImport->hasRestParameter()) {
        auto rest = m_callImport->getRestParameter();

        Parameter p;
        p.index = 0;
        p.name = rest.name;
        p.placement = rest.placement;
        p.isVariable = rest.isVariable;

        TypeHandle *paramType = nullptr;
        TU_ASSIGN_OR_RAISE (paramType, typeCache->importType(rest.type));
        p.typeDef = paramType->getTypeDef();

        priv->restParameter = Option(p);
        priv->parametersMap[p.name] = p;
    }

    priv->access = m_callImport->getAccess();
    priv->mode = m_callImport->getCallMode();
    priv->receiverUrl = m_callImport->getReceiverUrl();

//    auto *callType = m_callImport->getCallType();
//    TU_ASSIGN_OR_RAISE (priv->callType, typeCache->importType(callType));

    auto *callTemplate = m_callImport->getCallTemplate();
    if (callTemplate != nullptr) {
        TU_ASSIGN_OR_RAISE (priv->callTemplate, typeCache->importTemplate(callTemplate));
    }

    TypeHandle *returnType = nullptr;
    TU_ASSIGN_OR_RAISE (returnType, typeCache->importType(m_callImport->getReturnType()));
    priv->returnType = returnType->getTypeDef();

    priv->initializers = absl::flat_hash_map<std::string,lyric_common::SymbolUrl>(
        m_callImport->initializersBegin(), m_callImport->initializersEnd());
    auto *importCache = m_state->importCache();
    for (const auto &entry : priv->initializers) {
        TU_RAISE_IF_STATUS (importCache->importCall(entry.second));
    }

    bool importProc = false;
    switch (options->procImportMode) {
        case ProcImportMode::None:
            importProc = false;
            break;
        case ProcImportMode::InlineableOnly:
            importProc = priv->mode == lyric_object::CallMode::Inline;
            break;
        case ProcImportMode::All:
            importProc = true;
            break;
        default:
            break;
    }

    if (importProc) {
        auto moduleImport = m_callImport->getModuleImport();
        auto bytecode = m_callImport->getInlineBytecode();
        lyric_object::BytecodeIterator it(bytecode.data(), bytecode.size());
        TU_RAISE_IF_NOT_OK (internal::import_proc(moduleImport, m_callUrl, it, priv->proc, m_state));
    }

    return priv.release();
}

lyric_object::LinkageSection
lyric_assembler::CallSymbol::getLinkage() const
{
    return lyric_object::LinkageSection::Call;
}

lyric_assembler::SymbolType
lyric_assembler::CallSymbol::getSymbolType() const
{
    return SymbolType::CALL;
}

lyric_common::SymbolUrl
lyric_assembler::CallSymbol::getSymbolUrl() const
{
    return m_callUrl;
}

lyric_common::TypeDef
lyric_assembler::CallSymbol::getTypeDef() const
{
    return {};
}

tempo_utils::Result<lyric_assembler::ProcHandle *>
lyric_assembler::CallSymbol::defineCall(
    const ParameterPack &parameterPack,
    const lyric_common::TypeDef &returnType)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't put initializer on imported call {}", m_callUrl.toString());
    auto *priv = getPriv();

    if (priv->proc != nullptr)
        m_state->throwAssemblerInvariant("cannot redefine call {}", m_callUrl.toString());

    auto *symbolCache = m_state->symbolCache();
    auto *typeCache = m_state->typeCache();

    priv->listParameters = parameterPack.listParameters;
    priv->namedParameters = parameterPack.namedParameters;
    priv->restParameter = parameterPack.restParameter;
    priv->returnType = returnType;

    absl::flat_hash_map<std::string,SymbolBinding> bindings;

    // if call is bound, then synthesize the $this variable
    if (priv->receiverUrl.isValid()) {
        AbstractSymbol *receiverSymbol;
        TU_ASSIGN_OR_RAISE (receiverSymbol, symbolCache->getOrImportSymbol(priv->receiverUrl));
        auto thisPath = m_callUrl.getSymbolPath().getPath();
        thisPath.emplace_back("$this");
        auto thisUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(thisPath));
        auto thisType = receiverSymbol->getTypeDef();
        auto *thisSymbol = new SyntheticSymbol(thisUrl, SyntheticType::This, thisType);
        symbolCache->insertSymbol(thisUrl, thisSymbol);
        SymbolBinding thisBinding;
        thisBinding.symbolUrl = thisUrl;
        thisBinding.typeDef = thisType;
        thisBinding.bindingType = BindingType::Value;
        bindings["$this"] = thisBinding;
    }

    // create bindings for list parameters
    for (const auto &param : priv->listParameters) {
        auto paramPath = m_callUrl.getSymbolPath().getPath();
        paramPath.push_back(param.name);
        auto paramUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(paramPath));
        auto bindingType = param.isVariable? BindingType::Variable : BindingType::Value;
        ArgumentOffset offset(static_cast<tu_uint32>(param.index));
        auto *paramSymbol = new ArgumentVariable(paramUrl, param.typeDef, bindingType, offset);
        symbolCache->insertSymbol(paramUrl, paramSymbol);

        TU_RETURN_IF_STATUS (typeCache->getOrMakeType(param.typeDef));

        SymbolBinding argBinding;
        argBinding.symbolUrl = paramUrl;
        argBinding.typeDef = param.typeDef;
        argBinding.bindingType = bindingType;

        if (!param.label.empty()) {
            bindings[param.label] = argBinding;
        } else {
            bindings[param.name] = argBinding;
        }
        priv->parametersMap[param.name] = param;
    }

    // create bindings for named parameters
    for (const auto &param : priv->namedParameters) {
        auto paramPath = m_callUrl.getSymbolPath().getPath();
        paramPath.push_back(param.name);
        auto paramUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(paramPath));
        auto bindingType = param.isVariable? BindingType::Variable : BindingType::Value;
        ArgumentOffset offset(static_cast<tu_uint32>(param.index));
        auto *paramSymbol = new ArgumentVariable(paramUrl, param.typeDef, bindingType, offset);
        symbolCache->insertSymbol(paramUrl, paramSymbol);

        TU_RETURN_IF_STATUS (typeCache->getOrMakeType(param.typeDef));

        SymbolBinding argBinding;
        argBinding.symbolUrl = paramUrl;
        argBinding.typeDef = param.typeDef;
        argBinding.bindingType = bindingType;

        if (!param.label.empty()) {
            bindings[param.label] = argBinding;
        } else {
            bindings[param.name] = argBinding;
        }
        priv->parametersMap[param.name] = param;
    }

    // if there is a rest param then ensure the rest param type exists in the type cache
    std::string restParamName;
    lyric_common::TypeDef restParamType;
    if (!priv->restParameter.isEmpty()) {
        auto &param = priv->restParameter.peekValue();
        restParamName = param.name;
        TU_RETURN_IF_STATUS (typeCache->getOrMakeType(param.typeDef));
        restParamType = param.typeDef;
        // add the rest param to the parameters map, even if the param name is empty
        priv->parametersMap[restParamName] = param;
    }

    // if return type was explicitly declared then touch it
    if (priv->returnType.isValid()) {
        TU_RETURN_IF_STATUS (typeCache->getOrMakeType(priv->returnType));
    }

    // construct the proc handle
    priv->proc = std::make_unique<ProcHandle>(m_callUrl, bindings, priv->listParameters.size(),
        priv->namedParameters.size(), !priv->restParameter.isEmpty(), m_state, priv->parentBlock);

    // if call is generic then add the template parameters to the proc block
    if (priv->callTemplate != nullptr) {
        auto *callTemplate = priv->callTemplate;
        auto *procBlock = priv->proc->procBlock();
        for (auto it = callTemplate->templateParametersBegin(); it != callTemplate->templateParametersEnd(); it++) {
            const auto &tp = *it;
            TU_RAISE_IF_STATUS (procBlock->declareAlias(tp.name, callTemplate->getTemplateUrl(), tp.index));
        }
    }

    // if there is a named rest parameter then create a local for it
    if (!restParamName.empty()) {
        auto *fundamentalCache = m_state->fundamentalCache();
        auto fundamentalRestUrl = fundamentalCache->getFundamentalUrl(FundamentalSymbol::Rest);
        TypeHandle *restOfParamType;
        TU_ASSIGN_OR_RETURN (restOfParamType, typeCache->declareParameterizedType(
            fundamentalRestUrl, {restParamType}));

        auto *procBlock = priv->proc->procBlock();
        DataReference restVar;
        TU_ASSIGN_OR_RETURN (restVar, procBlock->declareVariable(restParamName, lyric_object::AccessType::Public,
            restOfParamType->getTypeDef(), /* isVariable= */ false));
    }

    return priv->proc.get();
}

std::string
lyric_assembler::CallSymbol::getName() const
{
    return m_callUrl.getSymbolPath().getName();
}

lyric_common::TypeDef
lyric_assembler::CallSymbol::getReturnType() const
{
    auto *priv = getPriv();
    return priv->returnType;
}

lyric_common::SymbolUrl
lyric_assembler::CallSymbol::getReceiverUrl() const
{
    auto *priv = getPriv();
    return priv->receiverUrl;
}

lyric_object::AccessType
lyric_assembler::CallSymbol::getAccessType() const
{
    auto *priv = getPriv();
    return priv->access;
}

lyric_object::CallMode
lyric_assembler::CallSymbol::getMode() const
{
    auto *priv = getPriv();
    return priv->mode;
}

bool
lyric_assembler::CallSymbol::isBound() const
{
    auto *priv = getPriv();
    return priv->receiverUrl.isValid();
}

bool
lyric_assembler::CallSymbol::isInline() const
{
    auto *priv = getPriv();
    return priv->mode == lyric_object::CallMode::Inline;
}

bool
lyric_assembler::CallSymbol::isCtor() const
{
    auto *priv = getPriv();
    return priv->mode == lyric_object::CallMode::Constructor;
}

bool
lyric_assembler::CallSymbol::isDeclOnly() const
{
    auto *priv = getPriv();
    return priv->isDeclOnly;
}

lyric_assembler::AbstractResolver *
lyric_assembler::CallSymbol::callResolver() const
{
    auto *priv = getPriv();
    if (priv->callTemplate != nullptr)
        return priv->callTemplate;
    return priv->parentBlock;
}

lyric_assembler::TypeHandle *
lyric_assembler::CallSymbol::callType()
{
    auto *priv = getPriv();
    if (priv->callType != nullptr)
        return priv->callType;

    std::vector<lyric_common::TypeDef> functionParameters;
    for (const auto &p : priv->listParameters) {
        functionParameters.push_back(p.typeDef);
    }
    for (const auto &p : priv->namedParameters) {
        functionParameters.push_back(p.typeDef);
    }

    Option<lyric_common::TypeDef> functionRest;
    if (priv->restParameter.hasValue()) {
        functionRest = Option(priv->restParameter.getValue().typeDef);
    }

    auto *typeCache = m_state->typeCache();
    TU_ASSIGN_OR_RAISE (priv->callType, typeCache->declareFunctionType(
        priv->returnType, functionParameters, functionRest));

    return priv->callType;
}

lyric_assembler::TemplateHandle *
lyric_assembler::CallSymbol::callTemplate() const
{
    auto *priv = getPriv();
    return priv->callTemplate;
}

const lyric_assembler::ProcHandle *
lyric_assembler::CallSymbol::callProc() const
{
    auto *priv = getPriv();
    return priv->proc.get();
}

lyric_assembler::ProcHandle *
lyric_assembler::CallSymbol::callProc()
{
    auto *priv = getPriv();
    return priv->proc.get();
}

bool
lyric_assembler::CallSymbol::hasParameter(const std::string name) const
{
    auto *priv = getPriv();
    return priv->parametersMap.contains(name);
}

lyric_assembler::Parameter
lyric_assembler::CallSymbol::getParameter(const std::string &name) const
{
    auto *priv = getPriv();
    auto entry = priv->parametersMap.find(name);
    if (entry == priv->parametersMap.cend())
        return {};
    return entry->second;
}

int
lyric_assembler::CallSymbol::numParameters() const
{
    auto *priv = getPriv();
    return priv->parametersMap.size();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::CallSymbol::listPlacementBegin() const
{
    auto *priv = getPriv();
    return priv->listParameters.cbegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::CallSymbol::listPlacementEnd() const
{
    auto *priv = getPriv();
    return priv->listParameters.cend();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::CallSymbol::namedPlacementBegin() const
{
    auto *priv = getPriv();
    return priv->namedParameters.cbegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::CallSymbol::namedPlacementEnd() const
{
    auto *priv = getPriv();
    return priv->namedParameters.cend();
}

const lyric_assembler::Parameter *
lyric_assembler::CallSymbol::restPlacement() const
{
    auto *priv = getPriv();
    if (priv->restParameter.isEmpty())
        return nullptr;
    return &priv->restParameter.peekValue();
}

bool
lyric_assembler::CallSymbol::hasInitializer(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->initializers.contains(name);
}

lyric_common::SymbolUrl
lyric_assembler::CallSymbol::getInitializer(const std::string &name) const
{
    auto *priv = getPriv();
    if (priv->initializers.contains(name))
        return priv->initializers.at(name);
    return {};
}

tempo_utils::Status
lyric_assembler::CallSymbol::putInitializer(
    const std::string &name,
    const lyric_common::SymbolUrl &initializerUrl)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't put initializer on imported call {}", m_callUrl.toString());
    auto *priv = getPriv();
    if (priv->initializers.contains(name))
        m_state->throwAssemblerInvariant(
            "initializer already defined for param {}", name);

    auto entry = priv->parametersMap.find(name);
    if (entry == priv->parametersMap.cend())
        return m_state->logAndContinue(AssemblerCondition::kInvalidBinding,
            tempo_tracing::LogSeverity::kError,
            "cannot put initializer for unknown param {}", name);

    priv->initializers[name] = initializerUrl;

    return {};
}

tempo_utils::Result<lyric_assembler::ProcHandle *>
lyric_assembler::CallSymbol::defineInitializer(const std::string &name)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't define initializer on imported call {}", m_callUrl.toString());
    auto *priv = getPriv();
    if (priv->initializers.contains(name))
        m_state->throwAssemblerInvariant(
            "initializer already defined for param {}", name);

    auto entry = priv->parametersMap.find(name);
    if (entry == priv->parametersMap.cend())
        return m_state->logAndContinue(AssemblerCondition::kInvalidBinding,
            tempo_tracing::LogSeverity::kError,
            "cannot define initializer for unknown param {}", name);
    auto &param = entry->second;

    auto identifier = absl::StrCat("$init$", name);

    // declare the initializer call
    lyric_assembler::CallSymbol *callSymbol;
    TU_ASSIGN_OR_RETURN (callSymbol, priv->parentBlock->declareFunction(
        identifier, lyric_object::AccessType::Public, {}, priv->isDeclOnly));

    priv->initializers[name] = callSymbol->getSymbolUrl();

    // define the initializer with no parameters and the param type as return type
    return callSymbol->defineCall({}, param.typeDef);
}

tempo_utils::Status
lyric_assembler::CallSymbol::finalizeCall()
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't put initializer on imported call {}", m_callUrl.toString());
    auto *priv = getPriv();

    // if return type was not explicitly defined then set it
    if (!priv->returnType.isValid()) {
        lyric_assembler::UnifiedTypeSet typeSet(m_state);
        for (auto it = priv->proc->exitTypesBegin(); it != priv->proc->exitTypesEnd(); it++) {
            TU_RETURN_IF_NOT_OK (typeSet.putType(*it));
        }
        priv->returnType = typeSet.getUnifiedType();
    }

    auto *typeCache = m_state->typeCache();

    // ensure return type exists in cache and is addressable
    TU_RETURN_IF_STATUS (typeCache->getOrMakeType(priv->returnType));

    return {};
}