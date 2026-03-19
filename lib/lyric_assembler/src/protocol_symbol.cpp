
#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/protocol_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

#include "lyric_assembler/existential_symbol.h"

lyric_assembler::ProtocolSymbol::ProtocolSymbol(
    const lyric_common::SymbolUrl &protocolUrl,
    bool isHidden,
    lyric_object::PortType port,
    lyric_object::CommunicationType comm,
    TypeHandle *protocolType,
    TypeHandle *sendType,
    TypeHandle *receiveType,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : BaseSymbol(new ProtocolSymbolPriv()),
      m_protocolUrl(protocolUrl),
      m_state(state)
{
    TU_ASSERT (m_protocolUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->isHidden = isHidden;
    priv->port = port;
    priv->comm = comm;
    priv->protocolType = protocolType;
    priv->sendType = sendType;
    priv->receiveType = receiveType;
    priv->parentBlock = parentBlock;
    priv->isDeclOnly = isDeclOnly;
    priv->protocolBlock = std::make_unique<BlockHandle>(protocolUrl, parentBlock);

    TU_ASSERT (priv->port != lyric_object::PortType::Invalid);
    TU_ASSERT (priv->comm != lyric_object::CommunicationType::Invalid);
    TU_ASSERT (priv->protocolType != nullptr);
    TU_ASSERT (priv->sendType != nullptr);
    TU_ASSERT (priv->receiveType != nullptr);
    TU_ASSERT (priv->parentBlock != nullptr);
}

lyric_assembler::ProtocolSymbol::ProtocolSymbol(
    const lyric_common::SymbolUrl &protocolUrl,
    lyric_importer::ProtocolImport *protocolImport,
    bool isCopied,
    ObjectState *state)
    : BaseSymbol(isCopied),
      m_protocolUrl(protocolUrl),
      m_protocolImport(protocolImport),
      m_state(state)
{
    TU_ASSERT (m_protocolUrl.isValid());
    TU_ASSERT (m_protocolImport != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::ProtocolSymbolPriv *
lyric_assembler::ProtocolSymbol::load()
{
    auto *typeCache = m_state->typeCache();

    auto priv = std::make_unique<ProtocolSymbolPriv>();

    priv->isDeclOnly = m_protocolImport->isDeclOnly();
    priv->isHidden = m_protocolImport->isHidden();

    priv->parentBlock = nullptr;
    priv->port = m_protocolImport->getPort();
    priv->comm = m_protocolImport->getCommunication();

    TU_ASSIGN_OR_RAISE (priv->protocolType, typeCache->importType(m_protocolImport->getProtocolType()));

    auto *sendType = m_protocolImport->getSendType();
    TU_ASSIGN_OR_RAISE (priv->sendType, typeCache->importType(sendType));
    auto *receiveType = m_protocolImport->getReceiveType();
    TU_ASSIGN_OR_RAISE (priv->receiveType, typeCache->importType(receiveType));

    priv->protocolBlock = std::make_unique<BlockHandle>(
        m_protocolUrl, absl::flat_hash_map<std::string, SymbolBinding>(), m_state);

    return priv.release();
}

lyric_object::LinkageSection
lyric_assembler::ProtocolSymbol::getLinkage() const
{
    return lyric_object::LinkageSection::Protocol;
}

lyric_assembler::SymbolType
lyric_assembler::ProtocolSymbol::getSymbolType() const
{
    return SymbolType::PROTOCOL;
}

lyric_common::SymbolUrl
lyric_assembler::ProtocolSymbol::getSymbolUrl() const
{
    return m_protocolUrl;
}

lyric_common::TypeDef
lyric_assembler::ProtocolSymbol::getTypeDef() const
{
    auto *priv = getPriv();
    return priv->protocolType->getTypeDef();
}

bool
lyric_assembler::ProtocolSymbol::isHidden() const
{
    auto *priv = getPriv();
    return priv->isHidden;
}

bool
lyric_assembler::ProtocolSymbol::isDeclOnly() const
{
    auto *priv = getPriv();
    return priv->isDeclOnly;
}

lyric_object::PortType
lyric_assembler::ProtocolSymbol::getPortType() const
{
    auto *priv = getPriv();
    return priv->port;
}

lyric_object::CommunicationType
lyric_assembler::ProtocolSymbol::getCommunicationType() const
{
    auto *priv = getPriv();
    return priv->comm;
}

lyric_assembler::TypeHandle *
lyric_assembler::ProtocolSymbol::protocolType() const
{
    auto *priv = getPriv();
    return priv->protocolType;
}

lyric_assembler::TypeHandle *
lyric_assembler::ProtocolSymbol::sendType() const
{
    auto *priv = getPriv();
    return priv->protocolType;
}

lyric_assembler::TypeHandle *
lyric_assembler::ProtocolSymbol::receiveType() const
{
    auto *priv = getPriv();
    return priv->protocolType;
}

lyric_assembler::BlockHandle *
lyric_assembler::ProtocolSymbol::protocolBlock() const
{
    auto *priv = getPriv();
    return priv->protocolBlock.get();
}

tempo_utils::Status
lyric_assembler::ProtocolSymbol::prepareMethod(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    CallableInvoker &invoker,
    bool thisReceiver) const
{
    auto *fundamentalCache = m_state->fundamentalCache();
    auto *symbolCache = m_state->symbolCache();

    auto protocolUrl = fundamentalCache->getFundamentalUrl(FundamentalSymbol::Protocol);
    ExistentialSymbol *protocolExistential;
    TU_ASSIGN_OR_RETURN (protocolExistential, symbolCache->getOrImportExistential(protocolUrl));
    return protocolExistential->prepareMethod(name, receiverType, invoker, false);
}
