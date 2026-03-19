
#include <lyric_importer/importer_result.h>
#include <lyric_importer/protocol_import.h>
#include <lyric_object/protocol_walker.h>

namespace lyric_importer {
    struct ProtocolImport::Priv {
        lyric_common::SymbolUrl symbolUrl;
        bool isDeclOnly = false;
        bool isHidden = false;
        lyric_object::PortType port = lyric_object::PortType::Invalid;
        lyric_object::CommunicationType comm = lyric_object::CommunicationType::Invalid;
        TypeImport *protocolType = nullptr;
        TypeImport *sendType = nullptr;
        TypeImport *receiveType = nullptr;
    };
}

lyric_importer::ProtocolImport::ProtocolImport(std::shared_ptr<ModuleImport> moduleImport, tu_uint32 protocolOffset)
    : BaseImport(moduleImport),
      m_protocolOffset(protocolOffset)
{
    TU_ASSERT (m_protocolOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::SymbolUrl
lyric_importer::ProtocolImport::getSymbolUrl()
{
    load();
    return m_priv->symbolUrl;
}

bool
lyric_importer::ProtocolImport::isDeclOnly()
{
    return m_priv->isDeclOnly;
}

bool
lyric_importer::ProtocolImport::isHidden()
{
    load();
    return m_priv->isHidden;
}

lyric_object::PortType
lyric_importer::ProtocolImport::getPort()
{
    load();
    return m_priv->port;
}

lyric_object::CommunicationType
lyric_importer::ProtocolImport::getCommunication()
{
    load();
    return m_priv->comm;
}

lyric_importer::TypeImport *
lyric_importer::ProtocolImport::getProtocolType()
{
    load();
    return m_priv->protocolType;
}

bool
lyric_importer::ProtocolImport::hasSendType()
{
    load();
    return m_priv->sendType != nullptr;
}

lyric_importer::TypeImport *
lyric_importer::ProtocolImport::getSendType()
{
    load();
    return m_priv->sendType;
}

bool
lyric_importer::ProtocolImport::hasReceiveType()
{
    load();
    return m_priv->receiveType != nullptr;
}

lyric_importer::TypeImport *
lyric_importer::ProtocolImport::getReceiveType()
{
    load();
    return m_priv->receiveType;
}

void
lyric_importer::ProtocolImport::load()
{
    absl::MutexLock locker(&m_lock);

    if (m_priv != nullptr)
        return;

    auto priv = std::make_unique<Priv>();

    auto moduleImport = getModuleImport();
    auto objectLocation = moduleImport->getObjectLocation();
    auto protocolWalker = moduleImport->getObject().getProtocol(m_protocolOffset);
    priv->symbolUrl = lyric_common::SymbolUrl(objectLocation, protocolWalker.getSymbolPath());

    priv->isDeclOnly = protocolWalker.isDeclOnly();

    switch (protocolWalker.getAccess()) {
        case lyric_object::AccessType::Hidden:
            priv->isHidden = true;
            break;
        case lyric_object::AccessType::Public:
            priv->isHidden = false;
            break;
        default:
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(
                    ImporterCondition::kImportError,
                    "cannot import protocol at index {} in module {}; invalid access type",
                    m_protocolOffset, objectLocation.toString()));
    }

    priv->port = protocolWalker.getPort();
    if (priv->port == lyric_object::PortType::Invalid)
        throw tempo_utils::StatusException(
            ImporterStatus::forCondition(
                ImporterCondition::kImportError,
                "cannot import protocol at index {} in module {}; invalid port type",
                m_protocolOffset, objectLocation.toString()));

    priv->comm = protocolWalker.getCommunication();
    if (priv->comm == lyric_object::CommunicationType::Invalid)
        throw tempo_utils::StatusException(
            ImporterStatus::forCondition(
                ImporterCondition::kImportError,
                "cannot import protocol at index {} in module {}; invalid communication type",
                m_protocolOffset, objectLocation.toString()));

    priv->protocolType = moduleImport->getType(protocolWalker.getProtocolType().getDescriptorOffset());

    auto sendTypeWalker = protocolWalker.getSendType();
    if (sendTypeWalker.isValid()) {
        priv->sendType = moduleImport->getType(sendTypeWalker.getDescriptorOffset());
    }

    auto receiveTypeWalker = protocolWalker.getReceiveType();
    if (receiveTypeWalker.isValid()) {
        priv->receiveType = moduleImport->getType(receiveTypeWalker.getDescriptorOffset());
    }

    m_priv = std::move(priv);
}
