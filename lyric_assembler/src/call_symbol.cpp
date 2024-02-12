
#include <lyric_assembler/argument_variable.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/synthetic_symbol.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>

/**
 * @brief Construct a call symbol for the $entry call.
 */
lyric_assembler::CallSymbol::CallSymbol(
    const lyric_common::SymbolUrl &entryUrl,
    const lyric_common::TypeDef &returnType,
    CallAddress address,
    TypeHandle *callType,
    AssemblyState *state)
    : BaseSymbol(address, new CallSymbolPriv()),
      m_callUrl(entryUrl),
      m_state(state)
{
    TU_ASSERT (m_callUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->returnType = returnType;
    priv->access = lyric_object::AccessType::Public;
    priv->mode = lyric_object::CallMode::Normal;
    priv->isNoReturn =  true;
    priv->callType = callType;
    priv->callTemplate = nullptr;
    priv->proc = std::make_unique<ProcHandle>(entryUrl, m_state);

    TU_ASSERT (priv->returnType.getType() == lyric_common::TypeDefType::NoReturn);
    TU_ASSERT (priv->callType != nullptr);
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
 * @param address
 * @param callType
 * @param callTemplate
 * @param parentBlock
 * @param state
 */
lyric_assembler::CallSymbol::CallSymbol(
    const lyric_common::SymbolUrl &callUrl,
    const std::vector<lyric_object::Parameter> &parameters,
    const Option<lyric_object::Parameter> &rest,
    const lyric_common::TypeDef &returnType,
    const lyric_common::SymbolUrl &receiverUrl,
    lyric_object::AccessType access,
    CallAddress address,
    lyric_object::CallMode mode,
    TypeHandle *callType,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : BaseSymbol(address, new CallSymbolPriv()),
      m_callUrl(callUrl),
      m_state(state)
{
    TU_ASSERT (m_callUrl.isValid());
    TU_ASSERT (parentBlock != nullptr);
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->parameters = parameters;
    priv->rest = rest;
    priv->returnType = returnType;
    priv->receiverUrl = receiverUrl;
    priv->access = access;
    priv->mode = mode;
    priv->callType = callType;
    priv->callTemplate = nullptr;

    TU_ASSERT (priv->receiverUrl.isValid());
    TU_ASSERT (priv->callType != nullptr);

    absl::flat_hash_map<std::string,SymbolBinding> bindings;

    // if call is bound, then synthesize the $this variable
    TU_ASSERT (m_state->symbolCache()->hasSymbol(receiverUrl));
    auto *receiverSymbol = m_state->symbolCache()->getSymbol(receiverUrl);
    auto thisPath = callUrl.getSymbolPath().getPath();
    thisPath.push_back("$this");
    auto thisUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(thisPath));
    auto thisType = receiverSymbol->getAssignableType();
    auto *thisSymbol = new SyntheticSymbol(thisUrl, SyntheticType::THIS, thisType);
    m_state->symbolCache()->insertSymbol(thisUrl, thisSymbol);
    SymbolBinding thisVar;
    thisVar.symbol = thisUrl;
    thisVar.type = thisType;
    thisVar.binding = lyric_parser::BindingType::VALUE;
    bindings["$this"] = thisVar;

    // instantiate argument variables for ctx params
    for (const auto &param : parameters) {

        auto paramPath = callUrl.getSymbolPath().getPath();
        paramPath.push_back(param.name);
        auto paramUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(paramPath));
        auto binding = param.isVariable? lyric_parser::BindingType::VARIABLE : lyric_parser::BindingType::VALUE;
        ArgumentOffset offset(static_cast<uint32_t>(param.index));
        auto *paramSymbol = new ArgumentVariable(paramUrl, param.typeDef, binding, offset);
        m_state->symbolCache()->insertSymbol(paramUrl, paramSymbol);

        SymbolBinding var;
        var.symbol = paramUrl;
        var.type = param.typeDef;
        var.binding = binding;

        if (!param.label.empty()) {
            bindings[param.label] = var;
        } else {
            bindings[param.name] = var;
        }
    }

    // FIXME: instantiate argument variable for collector param, if specified

    priv->proc = std::make_unique<ProcHandle>(callUrl, bindings, priv->parameters.size(), state, parentBlock);
}

/**
 * @brief Construct a CallSymbol for a bound method.
 *
 */
lyric_assembler::CallSymbol::CallSymbol(
    const lyric_common::SymbolUrl &callUrl,
    const std::vector<lyric_object::Parameter> &parameters,
    const Option<lyric_object::Parameter> &rest,
    const lyric_common::TypeDef &returnType,
    const lyric_common::SymbolUrl &receiverUrl,
    lyric_object::AccessType access,
    CallAddress address,
    lyric_object::CallMode mode,
    TypeHandle *callType,
    TemplateHandle *callTemplate,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : CallSymbol(
        callUrl,
        parameters,
        rest,
        returnType,
        receiverUrl,
        access,
        address,
        mode,
        callType,
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
 * @param address
 * @param callType
 * @param callTemplate
 * @param parentBlock
 * @param state
 */
lyric_assembler::CallSymbol::CallSymbol(
    const lyric_common::SymbolUrl &callUrl,
    const std::vector<lyric_object::Parameter> &parameters,
    const Option<lyric_object::Parameter> &rest,
    const lyric_common::TypeDef &returnType,
    lyric_object::AccessType access,
    CallAddress address,
    lyric_object::CallMode mode,
    TypeHandle *callType,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : BaseSymbol(address, new CallSymbolPriv()),
      m_callUrl(callUrl),
      m_state(state)
{
    TU_ASSERT (m_callUrl.isValid());
    TU_ASSERT (parentBlock != nullptr);
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->parameters = parameters;
    priv->rest = rest;
    priv->returnType = returnType;
    priv->access = access;
    priv->mode = mode;
    priv->callType = callType;
    priv->callTemplate = nullptr;

    TU_ASSERT (priv->callType != nullptr);

    absl::flat_hash_map<std::string,SymbolBinding> bindings;

    // instantiate argument variables for ctx params
    for (const auto &param : parameters) {

        auto paramPath = callUrl.getSymbolPath().getPath();
        paramPath.push_back(param.name);
        auto paramUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(paramPath));
        auto binding = param.isVariable? lyric_parser::BindingType::VARIABLE : lyric_parser::BindingType::VALUE;
        ArgumentOffset offset(static_cast<uint32_t>(param.index));
        auto *paramSymbol = new ArgumentVariable(paramUrl, param.typeDef, binding, offset);
        m_state->symbolCache()->insertSymbol(paramUrl, paramSymbol);

        SymbolBinding var;
        var.symbol = paramUrl;
        var.type = param.typeDef;
        var.binding = binding;

        if (!param.label.empty()) {
            bindings[param.label] = var;
        } else {
            bindings[param.name] = var;
        }
    }

    // FIXME: instantiate argument variable for collector param, if specified

    priv->proc = std::make_unique<ProcHandle>(callUrl, bindings, priv->parameters.size(), state, parentBlock);
}

/**
 * @brief Construct a CallSymbol for a free function.
 *
 */
lyric_assembler::CallSymbol::CallSymbol(
    const lyric_common::SymbolUrl &callUrl,
    const std::vector<lyric_object::Parameter> &parameters,
    const Option<lyric_object::Parameter> &rest,
    const lyric_common::TypeDef &returnType,
    lyric_object::AccessType access,
    CallAddress address,
    lyric_object::CallMode mode,
    TypeHandle *callType,
    TemplateHandle *callTemplate,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : CallSymbol(
        callUrl,
        parameters,
        rest,
        returnType,
        access,
        address,
        mode,
        callType,
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
    AssemblyState *state)
    : m_callUrl(callUrl),
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

    auto priv = std::make_unique<CallSymbolPriv>();

    for (auto it = m_callImport->parametersBegin(); it != m_callImport->parametersEnd(); it++) {
        lyric_object::Parameter p;
        p.index = it->index;
        p.name = it->name;
        p.label = it->label;
        p.isVariable = it->isVariable;
        p.placement = it->placement;

        TypeHandle *paramType = nullptr;
        TU_ASSIGN_OR_RAISE (paramType, typeCache->importType(it->type));
        p.typeDef = paramType->getTypeDef();

        priv->parameters.push_back(p);
    }

    if (m_callImport->hasRest()) {
        auto rest = m_callImport->getRest();

        lyric_object::Parameter p;
        p.index = rest.index;
        p.name = rest.name;
        p.label = rest.label;
        p.isVariable = rest.isVariable;
        p.placement = rest.placement;

        TypeHandle *paramType = nullptr;
        TU_ASSIGN_OR_RAISE (paramType, typeCache->importType(rest.type));
        p.typeDef = paramType->getTypeDef();

        priv->rest = Option(p);
    }

    priv->access = m_callImport->getAccess();
    priv->mode = m_callImport->getCallMode();
    priv->receiverUrl = m_callImport->getReceiverUrl();

    auto *callType = m_callImport->getCallType();
    TU_ASSIGN_OR_RAISE (priv->callType, typeCache->importType(callType));

    auto *callTemplate = m_callImport->getCallTemplate();
    if (callTemplate != nullptr) {
        TU_ASSIGN_OR_RAISE (priv->callTemplate, typeCache->importTemplate(callTemplate));
    }

    TypeHandle *returnType = nullptr;
    TU_ASSIGN_OR_RAISE (returnType, typeCache->importType(m_callImport->getReturnType()));
    priv->returnType = returnType->getTypeDef();

    priv->initializers = absl::flat_hash_map<std::string,lyric_common::SymbolUrl>(
        m_callImport->initializersBegin(), m_callImport->initializersEnd());

    if (priv->mode == lyric_object::CallMode::Inline) {
        auto bytecode = m_callImport->getInlineBytecode();
        priv->proc = std::make_unique<ProcHandle>(m_callUrl, bytecode);
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
lyric_assembler::CallSymbol::getAssignableType() const
{
    return lyric_common::TypeDef();
}

lyric_assembler::TypeSignature
lyric_assembler::CallSymbol::getTypeSignature() const
{
    return TypeSignature();
}

void
lyric_assembler::CallSymbol::touch()
{
    if (getAddress().isValid())
        return;
    m_state->touchCall(this);
}

std::vector<lyric_object::Parameter>
lyric_assembler::CallSymbol::getParameters() const
{
    auto *priv = getPriv();
    return priv->parameters;
}

Option<lyric_object::Parameter>
lyric_assembler::CallSymbol::getRest() const
{
    auto *priv = getPriv();
    return priv->rest;
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

lyric_assembler::TypeHandle *
lyric_assembler::CallSymbol::callType()
{
    auto *priv = getPriv();
    return priv->callType;
}

lyric_assembler::TemplateHandle *
lyric_assembler::CallSymbol::callTemplate()
{
    auto *priv = getPriv();
    return priv->callTemplate;
}

lyric_assembler::ProcHandle *
lyric_assembler::CallSymbol::callProc()
{
    auto *priv = getPriv();
    return priv->proc.get();
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

void
lyric_assembler::CallSymbol::putInitializer(const std::string &name, const lyric_common::SymbolUrl &initializer)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't put initializer on imported call {}", m_callUrl.toString());
    auto *priv = getPriv();
    priv->initializers[name] = initializer;
}

void
lyric_assembler::CallSymbol::putExitType(const lyric_common::TypeDef &exitType)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't put exit type on imported call {}", m_callUrl.toString());
    auto *priv = getPriv();
    priv->exitTypes.insert(exitType);
}

absl::flat_hash_set<lyric_common::TypeDef>
lyric_assembler::CallSymbol::listExitTypes() const
{
    auto *priv = getPriv();
    return priv->exitTypes;
}

std::vector<lyric_object::Parameter>::const_iterator
lyric_assembler::CallSymbol::placementBegin() const
{
    auto *priv = getPriv();
    return priv->parameters.cbegin();
}

std::vector<lyric_object::Parameter>::const_iterator
lyric_assembler::CallSymbol::placementEnd() const
{
    auto *priv = getPriv();
    return priv->parameters.cend();
}
