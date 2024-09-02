
#include <lyric_assembler/argument_variable.h>
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_importer/type_import.h>

lyric_assembler::ActionSymbol::ActionSymbol(
    const lyric_common::SymbolUrl &actionUrl,
    const lyric_common::SymbolUrl &receiverUrl,
    lyric_object::AccessType access,
    ActionAddress address,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : BaseSymbol(address, new ActionSymbolPriv()),
      m_actionUrl(actionUrl),
      m_state(state)
{
    TU_ASSERT (m_actionUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->isDeclOnly = isDeclOnly;
    priv->receiverUrl = receiverUrl;
    priv->access = access;
    priv->actionTemplate = nullptr;
    priv->parentBlock = parentBlock;

    TU_ASSERT (priv->receiverUrl.isValid());
    TU_ASSERT (priv->access != lyric_object::AccessType::Invalid);
    TU_ASSERT (priv->parentBlock != nullptr);
}

lyric_assembler::ActionSymbol::ActionSymbol(
    const lyric_common::SymbolUrl &actionUrl,
    const lyric_common::SymbolUrl &receiverUrl,
    lyric_object::AccessType access,
    ActionAddress address,
    TemplateHandle *actionTemplate,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : ActionSymbol(
        actionUrl,
        receiverUrl,
        access,
        address,
        isDeclOnly,
        parentBlock,
        state)
{
    auto *priv = getPriv();
    priv->actionTemplate = actionTemplate;
    TU_ASSERT (priv->actionTemplate != nullptr);
}

lyric_assembler::ActionSymbol::ActionSymbol(
    const lyric_common::SymbolUrl &actionUrl,
    lyric_importer::ActionImport *actionImport,
    ObjectState *state)
    : m_actionUrl(actionUrl),
      m_actionImport(actionImport),
      m_state(state)
{
    TU_ASSERT (m_actionUrl.isValid());
    TU_ASSERT (m_actionImport != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::ActionSymbolPriv *
lyric_assembler::ActionSymbol::load()
{
    auto *typeCache = m_state->typeCache();

    auto priv = std::make_unique<ActionSymbolPriv>();

    priv->isDeclOnly = m_actionImport->isDeclOnly();

    for (auto it = m_actionImport->listParametersBegin(); it != m_actionImport->listParametersEnd(); it++) {
        Parameter p;
        p.index = it->index;
        p.name = it->name;
        p.placement = it->placement;
        p.isVariable = it->isVariable;

        TypeHandle *paramType = nullptr;
        TU_ASSIGN_OR_RAISE (paramType, typeCache->importType(it->type));
        p.typeDef = paramType->getTypeDef();

        priv->listParameters.push_back(p);
    }

    for (auto it = m_actionImport->namedParametersBegin(); it != m_actionImport->namedParametersEnd(); it++) {
        Parameter p;
        p.index = it->index;
        p.name = it->name;
        p.placement = it->placement;
        p.isVariable = it->isVariable;

        TypeHandle *paramType = nullptr;
        TU_ASSIGN_OR_RAISE (paramType, typeCache->importType(it->type));
        p.typeDef = paramType->getTypeDef();

        priv->namedParameters.push_back(p);
    }

    if (m_actionImport->hasRestParameter()) {
        auto rest = m_actionImport->getRestParameter();

        Parameter p;
        p.index = 0;
        p.name = rest.name;
        p.placement = rest.placement;
        p.isVariable = rest.isVariable;

        TypeHandle *paramType = nullptr;
        TU_ASSIGN_OR_RAISE (paramType, typeCache->importType(rest.type));
        p.typeDef = paramType->getTypeDef();

        priv->restParameter = Option(p);
    }

    priv->receiverUrl = m_actionImport->getReceiverUrl();

    auto *actionTemplate = m_actionImport->getActionTemplate();
    if (actionTemplate != nullptr) {
        TU_ASSIGN_OR_RAISE (priv->actionTemplate, typeCache->importTemplate(actionTemplate));
    }

    TypeHandle *returnType = nullptr;
    TU_ASSIGN_OR_RAISE (returnType, typeCache->importType(m_actionImport->getReturnType()));
    priv->returnType = returnType->getTypeDef();

    priv->initializers = absl::flat_hash_map<std::string,lyric_common::SymbolUrl>(
        m_actionImport->initializersBegin(), m_actionImport->initializersEnd());

    return priv.release();
}

lyric_object::LinkageSection
lyric_assembler::ActionSymbol::getLinkage() const
{
    return lyric_object::LinkageSection::Action;
}

lyric_assembler::SymbolType
lyric_assembler::ActionSymbol::getSymbolType() const
{
    return SymbolType::ACTION;
}

lyric_common::SymbolUrl
lyric_assembler::ActionSymbol::getSymbolUrl() const
{
    return m_actionUrl;
}

lyric_common::TypeDef
lyric_assembler::ActionSymbol::getAssignableType() const
{
    return {};
}

bool
lyric_assembler::ActionSymbol::isDeclOnly() const
{
    auto *priv = getPriv();
    return priv->isDeclOnly;
}

tempo_utils::Status
lyric_assembler::ActionSymbol::defineAction(
    const ParameterPack &parameterPack,
    const lyric_common::TypeDef &returnType)
{
    auto *priv = getPriv();

    if (priv->returnType.isValid())
        m_state->throwAssemblerInvariant("cannot redefine action {}", m_actionUrl.toString());

    auto *typeCache = m_state->typeCache();

    priv->listParameters = parameterPack.listParameters;
    priv->namedParameters = parameterPack.namedParameters;
    priv->restParameter = parameterPack.restParameter;
    priv->returnType = returnType;

    for (const auto &param : priv->listParameters) {
        TU_RETURN_IF_STATUS (typeCache->getOrMakeType(param.typeDef));
    }

    for (const auto &param : priv->namedParameters) {
        TU_RETURN_IF_STATUS (typeCache->getOrMakeType(param.typeDef));
    }

    if (!priv->restParameter.isEmpty()) {
        auto &param = priv->restParameter.peekValue();
        TU_RETURN_IF_STATUS (typeCache->getOrMakeType(param.typeDef));
    }

    TU_RETURN_IF_STATUS (typeCache->getOrMakeType(priv->returnType));

    return {};
}

lyric_common::TypeDef
lyric_assembler::ActionSymbol::getReturnType() const
{
    auto *priv = getPriv();
    return priv->returnType;
}

lyric_common::SymbolUrl
lyric_assembler::ActionSymbol::getReceiverUrl() const
{
    auto *priv = getPriv();
    return priv->receiverUrl;
}

lyric_object::AccessType
lyric_assembler::ActionSymbol::getAccessType() const
{
    auto *priv = getPriv();
    return priv->access;
}

lyric_assembler::AbstractResolver *
lyric_assembler::ActionSymbol::actionResolver() const
{
    auto *priv = getPriv();
    if (priv->actionTemplate != nullptr)
        return priv->actionTemplate;
    return priv->parentBlock;
}

lyric_assembler::TemplateHandle *
lyric_assembler::ActionSymbol::actionTemplate() const
{
    auto *priv = getPriv();
    return priv->actionTemplate;
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ActionSymbol::listPlacementBegin() const
{
    auto *priv = getPriv();
    return priv->listParameters.cbegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ActionSymbol::listPlacementEnd() const
{
    auto *priv = getPriv();
    return priv->listParameters.cend();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ActionSymbol::namedPlacementBegin() const
{
    auto *priv = getPriv();
    return priv->namedParameters.cbegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ActionSymbol::namedPlacementEnd() const
{
    auto *priv = getPriv();
    return priv->namedParameters.cend();
}

const lyric_assembler::Parameter *
lyric_assembler::ActionSymbol::restPlacement() const
{
    auto *priv = getPriv();
    if (priv->restParameter.isEmpty())
        return nullptr;
    return &priv->restParameter.peekValue();
}

bool
lyric_assembler::ActionSymbol::hasInitializer(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->initializers.contains(name);
}

lyric_common::SymbolUrl
lyric_assembler::ActionSymbol::getInitializer(const std::string &name) const
{
    auto *priv = getPriv();
    if (priv->initializers.contains(name))
        return priv->initializers.at(name);
    return {};
}

void
lyric_assembler::ActionSymbol::putInitializer(const std::string &name, const lyric_common::SymbolUrl &initializer)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't put initializer on imported action {}", m_actionUrl.toString());
    auto *priv = getPriv();
    priv->initializers[name] = initializer;
}
