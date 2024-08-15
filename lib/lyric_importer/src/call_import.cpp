
#include <lyric_importer/call_import.h>
#include <lyric_importer/importer_result.h>
#include <lyric_importer/type_import.h>

namespace lyric_importer {
    struct CallImport::Priv {
        lyric_common::SymbolUrl symbolUrl;
        lyric_common::SymbolUrl receiverUrl;
        TemplateImport *callTemplate;
        TypeImport *returnType;
        bool isDeclOnly;
        lyric_object::AccessType access;
        lyric_object::CallMode callMode;
        std::vector<Parameter> listParameters;
        std::vector<Parameter> namedParameters;
        Option<Parameter> restParameter;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> initializers;
        std::vector<tu_uint8> inlineBytecode;
    };
}

lyric_importer::CallImport::CallImport(std::shared_ptr<ModuleImport> moduleImport, tu_uint32 callOffset)
    : m_moduleImport(moduleImport),
      m_callOffset(callOffset)
{
    TU_ASSERT (m_moduleImport != nullptr);
    TU_ASSERT (m_callOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::SymbolUrl
lyric_importer::CallImport::getSymbolUrl()
{
    load();
    return m_priv->symbolUrl;
}

bool
lyric_importer::CallImport::isDeclOnly()
{
    load();
    return m_priv->isDeclOnly;
}

lyric_object::AccessType
lyric_importer::CallImport::getAccess()
{
    load();
    return m_priv->access;
}

lyric_object::CallMode
lyric_importer::CallImport::getCallMode()
{
    load();
    return m_priv->callMode;
}

lyric_common::SymbolUrl
lyric_importer::CallImport::getReceiverUrl()
{
    load();
    return m_priv->receiverUrl;
}

lyric_importer::TemplateImport *
lyric_importer::CallImport::getCallTemplate()
{
    load();
    return m_priv->callTemplate;
}

lyric_importer::TypeImport *
lyric_importer::CallImport::getReturnType()
{
    load();
    return m_priv->returnType;
}

lyric_importer::Parameter
lyric_importer::CallImport::getListParameter(tu_uint8 index)
{
    load();
    if (index < m_priv->listParameters.size())
        return m_priv->listParameters.at(index);
    return {};
}

std::vector<lyric_importer::Parameter>::const_iterator
lyric_importer::CallImport::listParametersBegin()
{
    load();
    return m_priv->listParameters.cbegin();
}

std::vector<lyric_importer::Parameter>::const_iterator
lyric_importer::CallImport::listParametersEnd()
{
    load();
    return m_priv->listParameters.cend();
}

tu_uint8
lyric_importer::CallImport::numListParameters()
{
    load();
    return m_priv->listParameters.size();
}

lyric_importer::Parameter
lyric_importer::CallImport::getNamedParameter(tu_uint8 index)
{
    load();
    if (index < m_priv->namedParameters.size())
        return m_priv->namedParameters.at(index);
    return {};
}

std::vector<lyric_importer::Parameter>::const_iterator
lyric_importer::CallImport::namedParametersBegin()
{
    load();
    return m_priv->namedParameters.cbegin();
}

std::vector<lyric_importer::Parameter>::const_iterator
lyric_importer::CallImport::namedParametersEnd()
{
    load();
    return m_priv->namedParameters.cend();
}

tu_uint8
lyric_importer::CallImport::numNamedParameters()
{
    load();
    return m_priv->namedParameters.size();
}

bool
lyric_importer::CallImport::hasRestParameter()
{
    load();
    return !m_priv->restParameter.isEmpty();
}

lyric_importer::Parameter
lyric_importer::CallImport::getRestParameter()
{
    load();
    return m_priv->restParameter.getOrDefault({});
}

lyric_common::SymbolUrl
lyric_importer::CallImport::getInitializer(std::string_view name)
{
    load();
    if (m_priv->initializers.contains(name))
        return m_priv->initializers.at(name);
    return {};
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::CallImport::initializersBegin()
{
    load();
    return m_priv->initializers.cbegin();
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::CallImport::initializersEnd()
{
    load();
    return m_priv->initializers.cend();
}

tu_uint8
lyric_importer::CallImport::numInitializers()
{
    load();
    return m_priv->initializers.size();
}

std::vector<tu_uint8>
lyric_importer::CallImport::getInlineBytecode()
{
    load();
    return m_priv->inlineBytecode;
}

void
lyric_importer::CallImport::load()
{
    absl::MutexLock locker(&m_lock);

    if (m_priv != nullptr)
        return;

    auto priv = std::make_unique<Priv>();

    auto location = m_moduleImport->getLocation();
    auto callWalker = m_moduleImport->getObject().getObject().getCall(m_callOffset);
    priv->symbolUrl = lyric_common::SymbolUrl(location, callWalker.getSymbolPath());

    priv->isDeclOnly = callWalker.isDeclOnly();

    if (callWalker.hasReceiver()) {
        auto receiver = callWalker.getReceiver();

        switch (receiver.getLinkageSection()) {
            case lyric_object::LinkageSection::Class:
            case lyric_object::LinkageSection::Enum:
            case lyric_object::LinkageSection::Existential:
            case lyric_object::LinkageSection::Instance:
            case lyric_object::LinkageSection::Struct:
            {
                priv->receiverUrl = lyric_common::SymbolUrl(location, receiver.getSymbolPath());
                break;
            }
            default:
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(ImporterCondition::kImportError,
                        "cannot import call at index {} in module {}; invalid receiver",
                        callWalker.getDescriptorOffset(), location.toString()));
        }
    }

    if (callWalker.hasTemplate()) {
        priv->callTemplate = m_moduleImport->getTemplate(
            callWalker.getTemplate().getDescriptorOffset());
    } else {
        priv->callTemplate = nullptr;
    }

    priv->returnType = m_moduleImport->getType(callWalker.getResultType().getDescriptorOffset());

    tu_uint8 currparam = 0;

    // get list parameters
    for (tu_uint8 i = 0; i < callWalker.numListParameters(); i++) {
        auto parameter = callWalker.getListParameter(i);

        Parameter p;
        p.index = currparam++;
        p.name = parameter.getParameterName();
        p.type = m_moduleImport->getType(parameter.getParameterType().getDescriptorOffset());
        p.placement = parameter.getPlacement();
        p.isVariable = parameter.isVariable();


        if (p.placement == lyric_object::PlacementType::List) {
            if (parameter.hasInitializer())
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                        "cannot import call at index {} in module {}; invalid list parameter at index {}",
                        callWalker.getDescriptorOffset(), location.toString(), i));
        }
        else if (p.placement == lyric_object::PlacementType::ListOpt) {
            lyric_common::SymbolUrl initializerCallUrl;

            switch (parameter.initializerAddressType()) {
                case lyric_object::AddressType::Near: {
                    auto initializer = parameter.getNearInitializer();
                    if (!initializer.isValid())
                        throw tempo_utils::StatusException(
                            ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                                "cannot import call at index {} in module {}; invalid list parameter at index {}",
                                callWalker.getDescriptorOffset(), location.toString(), i));
                    initializerCallUrl = lyric_common::SymbolUrl(location, initializer.getSymbolPath());
                    break;
                }
                case lyric_object::AddressType::Far: {
                    auto initializer = parameter.getFarInitializer();
                    if (!initializer.isValid())
                        throw tempo_utils::StatusException(
                            ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                                "cannot import call at index {} in module {}; invalid list parameter at index {}",
                                callWalker.getDescriptorOffset(), location.toString(), i));
                    initializerCallUrl = initializer.getLinkUrl();
                    break;
                }
                default:
                    throw tempo_utils::StatusException(
                        ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                            "cannot import call at index {} in module {}; invalid list parameter at index {}",
                            callWalker.getDescriptorOffset(), location.toString(), i));
            }
            priv->initializers[p.name] = initializerCallUrl;
        }
        else {
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                    "cannot import call at index {} in module {}; invalid list parameter at index {}",
                    callWalker.getDescriptorOffset(), location.toString(), i));
        }

        priv->listParameters.push_back(p);
    }

    // get named parameters
    for (tu_uint8 i = 0; i < callWalker.numNamedParameters(); i++) {
        auto parameter = callWalker.getNamedParameter(i);

        Parameter p;
        p.index = currparam++;
        p.name = parameter.getParameterName();
        p.type = m_moduleImport->getType(parameter.getParameterType().getDescriptorOffset());
        p.placement = parameter.getPlacement();
        p.isVariable = parameter.isVariable();

        if (p.placement == lyric_object::PlacementType::Named || p.placement == lyric_object::PlacementType::Ctx) {
            if (parameter.hasInitializer())
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                        "cannot import call at index {} in module {}; invalid named parameter at index {}",
                        callWalker.getDescriptorOffset(), location.toString(), i));
        }
        else if (p.placement == lyric_object::PlacementType::NamedOpt) {
            lyric_common::SymbolUrl initializerCallUrl;

            switch (parameter.initializerAddressType()) {
                case lyric_object::AddressType::Near: {
                    auto initializer = parameter.getNearInitializer();
                    if (!initializer.isValid())
                        throw tempo_utils::StatusException(
                            ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                                "cannot import call at index {} in module {}; invalid named parameter at index {}",
                                callWalker.getDescriptorOffset(), location.toString(), i));
                    initializerCallUrl = lyric_common::SymbolUrl(
                        location, parameter.getNearInitializer().getSymbolPath());
                    break;
                }
                case lyric_object::AddressType::Far: {
                    auto initializer = parameter.getFarInitializer();
                    if (!initializer.isValid())
                        throw tempo_utils::StatusException(
                            ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                                "cannot import call at index {} in module {}; invalid named parameter at index {}",
                                callWalker.getDescriptorOffset(), location.toString(), i));
                    initializerCallUrl = parameter.getFarInitializer().getLinkUrl();
                    break;
                }
                default:
                    throw tempo_utils::StatusException(
                        ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                            "cannot import call at index {} in module {}; invalid named parameter at index {}",
                            callWalker.getDescriptorOffset(), location.toString(), i));
            }
            priv->initializers[p.name] = initializerCallUrl;
        }
        else {
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                    "cannot import call at index {} in module {}; invalid named parameter at index {}",
                    callWalker.getDescriptorOffset(), location.toString(), i));
        }

        priv->namedParameters.push_back(p);
    }

    if (callWalker.hasRestParameter()) {
        auto parameter = callWalker.getRestParameter();

        Parameter p;
        p.index = -1;
        p.name = parameter.getParameterName();
        p.type = m_moduleImport->getType(parameter.getParameterType().getDescriptorOffset());
        p.placement = parameter.getPlacement();
        p.isVariable = parameter.isVariable();

        if (p.placement != lyric_object::PlacementType::Rest)
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(ImporterCondition::kImportError,
                    "cannot import call at index {} in module {}; invalid rest parameter",
                    callWalker.getDescriptorOffset(), location.toString()));
        if (parameter.hasInitializer())
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(ImporterCondition::kImportError,
                    "cannot import call at index {} in module {}; invalid rest parameter",
                    callWalker.getDescriptorOffset(), location.toString()));

        priv->restParameter = Option(p);
    }

    if (callWalker.getAccess() == lyric_object::AccessType::Invalid)
        throw tempo_utils::StatusException(
            ImporterStatus::forCondition(
                ImporterCondition::kImportError,
                "cannot import call at index {} in module {}; invalid access type",
                m_callOffset, location.toString()));

    priv->access = callWalker.getAccess();

    if (callWalker.getMode() == lyric_object::CallMode::Invalid)
        throw tempo_utils::StatusException(
            ImporterStatus::forCondition(
                ImporterCondition::kImportError,
                "cannot import call at index {} in module {}; invalid call mode",
                m_callOffset, location.toString()));

    priv->callMode = callWalker.getMode();

    if (priv->callMode == lyric_object::CallMode::Inline) {
        auto procHeader = callWalker.getProcHeader();

        if (procHeader.numLocals != 0)
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                    "cannot import call at index {} in module {}; invalid inline proc",
                    callWalker.getDescriptorOffset(), location.toString()));
        if (procHeader.numLexicals != 0)
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                    "cannot import call at index {} in module {}; invalid inline proc",
                    callWalker.getDescriptorOffset(), location.toString()));
        auto it = callWalker.getBytecodeIterator();
        lyric_object::OpCell cell;
        while (it.getNext(cell)) {
            if (cell.opcode == lyric_object::Opcode::OP_RETURN)
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                        "cannot import call at index {} in module {}; invalid inline proc",
                        callWalker.getDescriptorOffset(), location.toString()));
        }

        priv->inlineBytecode = std::vector<tu_uint8>(it.getBase(), it.getCanary());
    }

    m_priv = std::move(priv);
}
