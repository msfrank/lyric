
#include <lyric_importer/action_import.h>
#include <lyric_importer/importer_result.h>
#include <lyric_importer/type_import.h>

namespace lyric_importer {
    struct ActionImport::Priv {
        lyric_common::SymbolUrl symbolUrl;
        lyric_common::SymbolUrl receiverUrl;
        TemplateImport *actionTemplate;
        TypeImport *returnType;
        std::vector<Parameter> parameters;
        Option<Parameter> rest;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> initializers;
    };
}

lyric_importer::ActionImport::ActionImport(
    std::shared_ptr<ModuleImport> moduleImport,
    tu_uint32 actionOffset)
    : m_moduleImport(moduleImport),
      m_actionOffset(actionOffset)
{
    TU_ASSERT (m_moduleImport != nullptr);
    TU_ASSERT (m_actionOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::SymbolUrl
lyric_importer::ActionImport::getSymbolUrl()
{
    load();
    return m_priv->symbolUrl;
}

lyric_common::SymbolUrl
lyric_importer::ActionImport::getReceiverUrl()
{
    load();
    return m_priv->receiverUrl;
}

lyric_importer::TemplateImport *
lyric_importer::ActionImport::getActionTemplate()
{
    load();
    return m_priv->actionTemplate;
}

lyric_importer::TypeImport *
lyric_importer::ActionImport::getReturnType()
{
    load();
    return m_priv->returnType;
}

lyric_importer::Parameter
lyric_importer::ActionImport::getParameter(tu_uint8 index)
{
    load();
    if (index < m_priv->parameters.size())
        return m_priv->parameters.at(index);
    return {};
}

std::vector<lyric_importer::Parameter>::const_iterator
lyric_importer::ActionImport::parametersBegin()
{
    load();
    return m_priv->parameters.cbegin();
}

std::vector<lyric_importer::Parameter>::const_iterator
lyric_importer::ActionImport::parametersEnd()
{
    load();
    return m_priv->parameters.cend();
}

tu_uint8
lyric_importer::ActionImport::numParameters()
{
    load();
    return m_priv->parameters.size();
}

bool
lyric_importer::ActionImport::hasRest()
{
    load();
    return !m_priv->rest.isEmpty();
}

lyric_importer::Parameter
lyric_importer::ActionImport::getRest()
{
    load();
    return m_priv->rest.getOrDefault({});
}

lyric_common::SymbolUrl
lyric_importer::ActionImport::getInitializer(std::string_view name)
{
    load();
    if (m_priv->initializers.contains(name))
        return m_priv->initializers.at(name);
    return {};
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::ActionImport::initializersBegin()
{
    load();
    return m_priv->initializers.cbegin();
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::ActionImport::initializersEnd()
{
    load();
    return m_priv->initializers.cend();
}

tu_uint8
lyric_importer::ActionImport::numInitializers()
{
    load();
    return m_priv->initializers.size();
}

void
lyric_importer::ActionImport::load()
{
    absl::MutexLock locker(&m_lock);

    if (m_priv != nullptr)
        return;

    auto priv = std::make_unique<Priv>();

    auto location = m_moduleImport->getLocation();
    auto actionWalker = m_moduleImport->getObject().getObject().getAction(m_actionOffset);
    priv->symbolUrl = lyric_common::SymbolUrl(location, actionWalker.getSymbolPath());

    auto receiver = actionWalker.getReceiver();
    switch (receiver.getLinkageSection()) {
        case lyric_object::LinkageSection::Concept:
        {
            priv->receiverUrl = lyric_common::SymbolUrl(location, receiver.getSymbolPath());
            break;
        }
        default:
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(ImporterCondition::kImportError,
                    "cannot import action at index {} in assembly {}; invalid receiver",
                    actionWalker.getDescriptorOffset(), location.toString()));
    }

    if (actionWalker.hasTemplate()) {
        priv->actionTemplate = m_moduleImport->getTemplate(
            actionWalker.getTemplate().getDescriptorOffset());
    } else {
        priv->actionTemplate = nullptr;
    }

    priv->returnType = m_moduleImport->getType(actionWalker.getResultType().getDescriptorOffset());

    // get call parameters
    for (tu_uint8 i = 0; i < actionWalker.numParameters(); i++) {
        auto parameter = actionWalker.getParameter(i);

        Parameter p;
        p.index = i;
        p.name = parameter.getParameterName();
        p.label = parameter.getParameterLabel();
        p.type = m_moduleImport->getType(parameter.getParameterType().getDescriptorOffset());
        p.placement = parameter.getPlacement();
        p.isVariable = parameter.isVariable();

        if (p.placement == lyric_object::PlacementType::Opt) {
            lyric_common::SymbolUrl initializerCallUrl;

            switch (parameter.initializerAddressType()) {
                case lyric_object::AddressType::Near: {
                    initializerCallUrl = lyric_common::SymbolUrl(
                        location, parameter.getNearInitializer().getSymbolPath());
                    break;
                }
                case lyric_object::AddressType::Far: {
                    initializerCallUrl = parameter.getFarInitializer().getLinkUrl();
                    break;
                }
                default:
                    throw tempo_utils::StatusException(
                        ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                            "cannot import action at index {} in assembly {}; invalid parameter at index {}",
                            actionWalker.getDescriptorOffset(), location.toString(), i));
            }

            priv->initializers[p.name] = initializerCallUrl;
        }

        priv->parameters.push_back(p);
    }

    if (actionWalker.hasRest()) {
        auto parameter = actionWalker.getRest();

        Parameter p;
        p.index = -1;
        p.name = parameter.getParameterName();
        p.label = {};
        p.type = m_moduleImport->getType(parameter.getParameterType().getDescriptorOffset());
        p.placement = parameter.getPlacement();
        p.isVariable = parameter.isVariable();

        if (p.placement != lyric_object::PlacementType::Rest)
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(ImporterCondition::kImportError,
                    "cannot import action at index {} in assembly {}; invalid rest parameter",
                    actionWalker.getDescriptorOffset(), location.toString()));
        // FIXME: need additional validation? (can't have initializer, etc)

        priv->rest = Option(p);
    }

    m_priv = std::move(priv);
}