
#include <lyric_importer/action_import.h>
#include <lyric_importer/importer_result.h>
#include <lyric_importer/type_import.h>

namespace lyric_importer {
    struct ActionImport::Priv {
        lyric_common::SymbolUrl symbolUrl;
        lyric_common::SymbolUrl receiverUrl;
        bool isDeclOnly;
        lyric_object::AccessType access;
        TemplateImport *actionTemplate;
        TypeImport *returnType;
        std::vector<Parameter> listParameters;
        std::vector<Parameter> namedParameters;
        Option<Parameter> restParameter;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> initializers;
    };
}

lyric_importer::ActionImport::ActionImport(
    std::shared_ptr<ModuleImport> moduleImport,
    tu_uint32 actionOffset)
    : BaseImport(moduleImport),
      m_actionOffset(actionOffset)
{
    TU_ASSERT (m_actionOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::SymbolUrl
lyric_importer::ActionImport::getSymbolUrl()
{
    load();
    return m_priv->symbolUrl;
}

bool
lyric_importer::ActionImport::isDeclOnly()
{
    load();
    return m_priv->isDeclOnly;
}

lyric_object::AccessType
lyric_importer::ActionImport::getAccess()
{
    load();
    return m_priv->access;
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
lyric_importer::ActionImport::getListParameter(tu_uint8 index)
{
    load();
    if (index < m_priv->listParameters.size())
        return m_priv->listParameters.at(index);
    return {};
}

std::vector<lyric_importer::Parameter>::const_iterator
lyric_importer::ActionImport::listParametersBegin()
{
    load();
    return m_priv->listParameters.cbegin();
}

std::vector<lyric_importer::Parameter>::const_iterator
lyric_importer::ActionImport::listParametersEnd()
{
    load();
    return m_priv->listParameters.cend();
}

tu_uint8
lyric_importer::ActionImport::numListParameters()
{
    load();
    return m_priv->listParameters.size();
}

lyric_importer::Parameter
lyric_importer::ActionImport::getNamedParameter(tu_uint8 index)
{
    load();
    if (index < m_priv->namedParameters.size())
        return m_priv->namedParameters.at(index);
    return {};
}

std::vector<lyric_importer::Parameter>::const_iterator
lyric_importer::ActionImport::namedParametersBegin()
{
    load();
    return m_priv->namedParameters.cbegin();
}

std::vector<lyric_importer::Parameter>::const_iterator
lyric_importer::ActionImport::namedParametersEnd()
{
    load();
    return m_priv->namedParameters.cend();
}

tu_uint8
lyric_importer::ActionImport::numNamedParameters()
{
    load();
    return m_priv->namedParameters.size();
}

bool
lyric_importer::ActionImport::hasRestParameter()
{
    load();
    return !m_priv->restParameter.isEmpty();
}

lyric_importer::Parameter
lyric_importer::ActionImport::getRestParameter()
{
    load();
    return m_priv->restParameter.getOrDefault({});
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

    auto moduleImport = getModuleImport();
    auto location = moduleImport->getLocation();
    auto actionWalker = moduleImport->getObject().getObject().getAction(m_actionOffset);
    priv->symbolUrl = lyric_common::SymbolUrl(location, actionWalker.getSymbolPath());

    priv->isDeclOnly = actionWalker.isDeclOnly();

    priv->access = actionWalker.getAccess();
    if (priv->access == lyric_object::AccessType::Invalid)
        throw tempo_utils::StatusException(
            ImporterStatus::forCondition(
                ImporterCondition::kImportError,
                "cannot import action at index {} in module {}; invalid access type",
                m_actionOffset, location.toString()));

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
                    "cannot import action at index {} in module {}; invalid receiver",
                    actionWalker.getDescriptorOffset(), location.toString()));
    }

    if (actionWalker.hasTemplate()) {
        priv->actionTemplate = moduleImport->getTemplate(
            actionWalker.getTemplate().getDescriptorOffset());
    } else {
        priv->actionTemplate = nullptr;
    }

    priv->returnType = moduleImport->getType(actionWalker.getResultType().getDescriptorOffset());

    tu_uint8 currparam = 0;

    // get list parameters
    for (tu_uint8 i = 0; i < actionWalker.numListParameters(); i++) {
        auto parameter = actionWalker.getListParameter(i);

        Parameter p;
        p.index = currparam++;
        p.name = parameter.getParameterName();
        p.type = moduleImport->getType(parameter.getParameterType().getDescriptorOffset());
        p.placement = parameter.getPlacement();
        p.isVariable = parameter.isVariable();

        if (p.placement == lyric_object::PlacementType::List) {
            if (parameter.hasInitializer())
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                        "cannot import action at index {} in module {}; invalid list parameter at index {}",
                        actionWalker.getDescriptorOffset(), location.toString(), i));
        }
        else if (p.placement == lyric_object::PlacementType::ListOpt) {
            lyric_common::SymbolUrl initializerCallUrl;

            switch (parameter.initializerAddressType()) {
                case lyric_object::AddressType::Near: {
                    auto initializer = parameter.getNearInitializer();
                    if (!initializer.isValid())
                        throw tempo_utils::StatusException(
                            ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                                "cannot import action at index {} in module {}; invalid list parameter at index {}",
                                actionWalker.getDescriptorOffset(), location.toString(), i));
                    initializerCallUrl = lyric_common::SymbolUrl(location, initializer.getSymbolPath());
                    break;
                }
                case lyric_object::AddressType::Far: {
                    auto initializer = parameter.getFarInitializer();
                    if (!initializer.isValid())
                        throw tempo_utils::StatusException(
                            ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                                "cannot import action at index {} in module {}; invalid list parameter at index {}",
                                actionWalker.getDescriptorOffset(), location.toString(), i));
                    initializerCallUrl = initializer.getLinkUrl();
                    break;
                }
                default:
                    throw tempo_utils::StatusException(
                        ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                            "cannot import action at index {} in module {}; invalid list parameter at index {}",
                            actionWalker.getDescriptorOffset(), location.toString(), i));
            }
            priv->initializers[p.name] = initializerCallUrl;
        }
        else {
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                    "cannot import action at index {} in module {}; invalid list parameter at index {}",
                    actionWalker.getDescriptorOffset(), location.toString(), i));
        }

        priv->listParameters.push_back(p);
    }

    // get call parameters
    for (tu_uint8 i = 0; i < actionWalker.numNamedParameters(); i++) {
        auto parameter = actionWalker.getNamedParameter(i);

        Parameter p;
        p.index = currparam++;
        p.name = parameter.getParameterName();
        p.type = moduleImport->getType(parameter.getParameterType().getDescriptorOffset());
        p.placement = parameter.getPlacement();
        p.isVariable = parameter.isVariable();

        if (p.placement == lyric_object::PlacementType::Named) {
            if (parameter.hasInitializer())
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                        "cannot import action at index {} in module {}; invalid named parameter at index {}",
                        actionWalker.getDescriptorOffset(), location.toString(), i));
        }
        else if (p.placement == lyric_object::PlacementType::NamedOpt) {
            lyric_common::SymbolUrl initializerCallUrl;

            switch (parameter.initializerAddressType()) {
                case lyric_object::AddressType::Near: {
                    auto initializer = parameter.getNearInitializer();
                    if (!initializer.isValid())
                        throw tempo_utils::StatusException(
                            ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                                "cannot import action at index {} in module {}; invalid named parameter at index {}",
                                actionWalker.getDescriptorOffset(), location.toString(), i));
                    initializerCallUrl = lyric_common::SymbolUrl(location, initializer.getSymbolPath());
                    break;
                }
                case lyric_object::AddressType::Far: {
                    auto initializer = parameter.getFarInitializer();
                    if (!initializer.isValid())
                        throw tempo_utils::StatusException(
                            ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                                "cannot import action at index {} in module {}; invalid named parameter at index {}",
                                actionWalker.getDescriptorOffset(), location.toString(), i));
                    initializerCallUrl = initializer.getLinkUrl();
                    break;
                }
                default:
                    throw tempo_utils::StatusException(
                        ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                            "cannot import action at index {} in module {}; invalid named parameter at index {}",
                            actionWalker.getDescriptorOffset(), location.toString(), i));
            }
            priv->initializers[p.name] = initializerCallUrl;
        }
        else {
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                    "cannot import action at index {} in module {}; invalid named parameter at index {}",
                    actionWalker.getDescriptorOffset(), location.toString(), i));
        }

        priv->namedParameters.push_back(p);
    }

    if (actionWalker.hasRestParameter()) {
        auto parameter = actionWalker.getRestParameter();

        Parameter p;
        p.index = -1;
        p.name = parameter.getParameterName();
        p.type = moduleImport->getType(parameter.getParameterType().getDescriptorOffset());
        p.placement = parameter.getPlacement();
        p.isVariable = parameter.isVariable();

        if (p.placement != lyric_object::PlacementType::Rest)
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(ImporterCondition::kImportError,
                    "cannot import action at index {} in module {}; invalid rest parameter",
                    actionWalker.getDescriptorOffset(), location.toString()));
        if (parameter.hasInitializer())
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(ImporterCondition::kImportError,
                    "cannot import action at index {} in module {}; invalid rest parameter",
                    actionWalker.getDescriptorOffset(), location.toString()));

        priv->restParameter = Option(p);
    }

    m_priv = std::move(priv);
}