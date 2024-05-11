
#include <lyric_assembler/argument_variable.h>
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_importer/type_import.h>

lyric_assembler::ActionSymbol::ActionSymbol(
    const lyric_common::SymbolUrl &actionUrl,
    const std::vector<lyric_object::Parameter> &parameters,
    const Option<lyric_object::Parameter> &rest,
    const lyric_common::TypeDef &returnType,
    const lyric_common::SymbolUrl &receiverUrl,
    ActionAddress address,
    AssemblyState *state)
    : BaseSymbol(address, new ActionSymbolPriv()),
      m_actionUrl(actionUrl),
      m_state(state)
{
    TU_ASSERT (m_actionUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->parameters = parameters;
    priv->rest = rest;
    priv->returnType = returnType;
    priv->receiverUrl = receiverUrl;
    priv->actionTemplate = nullptr;
}

lyric_assembler::ActionSymbol::ActionSymbol(
    const lyric_common::SymbolUrl &actionUrl,
    const std::vector<lyric_object::Parameter> &parameters,
    const Option<lyric_object::Parameter> &rest,
    const lyric_common::TypeDef &returnType,
    const lyric_common::SymbolUrl &receiverUrl,
    ActionAddress address,
    TemplateHandle *actionTemplate,
    AssemblyState *state)
    : ActionSymbol(
        actionUrl,
        parameters,
        rest,
        returnType,
        receiverUrl,
        address,
        state)
{
    auto *priv = getPriv();
    priv->actionTemplate = actionTemplate;
    TU_ASSERT (priv->actionTemplate != nullptr);
}

lyric_assembler::ActionSymbol::ActionSymbol(
    const lyric_common::SymbolUrl &actionUrl,
    lyric_importer::ActionImport *actionImport,
    AssemblyState *state)
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

    for (auto it = m_actionImport->parametersBegin(); it != m_actionImport->parametersEnd(); it++) {
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

    if (m_actionImport->hasRest()) {
        auto rest = m_actionImport->getRest();

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
    return lyric_common::TypeDef();
}

lyric_assembler::TypeSignature
lyric_assembler::ActionSymbol::getTypeSignature() const
{
    return TypeSignature();
}

void
lyric_assembler::ActionSymbol::touch()
{
    if (getAddress().isValid())
        return;
    m_state->touchAction(this);
}

std::vector<lyric_object::Parameter>
lyric_assembler::ActionSymbol::getParameters() const
{
    auto *priv = getPriv();
    return priv->parameters;
}

Option<lyric_object::Parameter>
lyric_assembler::ActionSymbol::getRest() const
{
    auto *priv = getPriv();
    return priv->rest;
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

lyric_assembler::TemplateHandle *
lyric_assembler::ActionSymbol::actionTemplate()
{
    auto *priv = getPriv();
    return priv->actionTemplate;
}

std::vector<lyric_object::Parameter>::const_iterator
lyric_assembler::ActionSymbol::placementBegin() const
{
    auto *priv = getPriv();
    return priv->parameters.cbegin();
}

std::vector<lyric_object::Parameter>::const_iterator
lyric_assembler::ActionSymbol::placementEnd() const
{
    auto *priv = getPriv();
    return priv->parameters.cend();
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
