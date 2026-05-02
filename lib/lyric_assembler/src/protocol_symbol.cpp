
#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/protocol_symbol.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

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
    std::shared_ptr<lyric_importer::ProtocolImport> protocolImport,
    bool isCopied,
    ObjectState *state)
    : BaseSymbol(isCopied),
      m_protocolUrl(protocolUrl),
      m_protocolImport(std::move(protocolImport)),
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

    auto typeImport = m_protocolImport->getProtocolType().lock();
    if (typeImport == nullptr)
        throw tempo_utils::StatusException(
            AssemblerStatus::forCondition(AssemblerCondition::kImportError,
            "cannot import protocol {}; missing type",
            m_protocolUrl.toString()));
    TU_ASSIGN_OR_RAISE (priv->protocolType, typeCache->importType(typeImport));

    auto sendImport = m_protocolImport->getSendType().lock();
    if (sendImport == nullptr)
        throw tempo_utils::StatusException(
            AssemblerStatus::forCondition(AssemblerCondition::kImportError,
            "cannot import protocol {}; missing send type",
            m_protocolUrl.toString()));
    TU_ASSIGN_OR_RAISE (priv->sendType, typeCache->importType(sendImport));

    auto receiveImport = m_protocolImport->getReceiveType().lock();
    if (receiveImport == nullptr)
        throw tempo_utils::StatusException(
            AssemblerStatus::forCondition(AssemblerCondition::kImportError,
            "cannot import protocol {}; missing receive type",
            m_protocolUrl.toString()));
    TU_ASSIGN_OR_RAISE (priv->receiveType, typeCache->importType(receiveImport));

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

lyric_assembler::BlockHandle *
lyric_assembler::ProtocolSymbol::derefBlock()
{
    auto *priv = getPriv();
    return priv->protocolBlock.get();
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
    return priv->sendType;
}

lyric_assembler::TypeHandle *
lyric_assembler::ProtocolSymbol::receiveType() const
{
    auto *priv = getPriv();
    return priv->receiveType;
}

lyric_assembler::BlockHandle *
lyric_assembler::ProtocolSymbol::protocolBlock() const
{
    auto *priv = getPriv();
    return priv->protocolBlock.get();
}


tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::ProtocolSymbol::resolveGlobalMember(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver) const
{
    auto *symbolCache = m_state->symbolCache();
    auto *priv = getPriv();

    auto globalSymbolUrl = priv->protocolBlock->makeSymbolUrl(name);

    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(globalSymbolUrl, /* allowMissing= */ true));
    if (symbol == nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kMissingMember,
            "missing global member {}", name);

    DataReference ref;
    bool isHidden;
    switch (symbol->getSymbolType()) {
        case SymbolType::ENUM: {
            auto *enumSymbol = cast_symbol_to_enum(symbol);
            ref.referenceType = ReferenceType::Value;
            isHidden = enumSymbol->isHidden();
            break;
        }
        case SymbolType::INSTANCE: {
            auto *instanceSymbol = cast_symbol_to_instance(symbol);
            ref.referenceType = ReferenceType::Value;
            isHidden = instanceSymbol->isHidden();
            break;
        }
        case SymbolType::STATIC: {
            auto *staticSymbol = cast_symbol_to_static(symbol);
            ref.referenceType = staticSymbol->isVariable()? ReferenceType::Variable : ReferenceType::Value;
            isHidden = staticSymbol->isHidden();
            break;
        }
        default:
            return AssemblerStatus::forCondition(AssemblerCondition::kMissingMember,
                "missing member {}", name);
    }

    bool thisSymbol = receiverType.getConcreteUrl() == m_protocolUrl;
    if (isHidden && !(thisReceiver && thisSymbol))
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
            "access to hidden member {} is not allowed", name);
    ref.symbolUrl = globalSymbolUrl;
    ref.typeDef = symbol->getTypeDef();
    return ref;
}

tempo_utils::Status
lyric_assembler::ProtocolSymbol::prepareGlobalMethod(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    CallableInvoker &invoker,
    bool thisReceiver) const
{
    auto *symbolCache = m_state->symbolCache();
    auto *priv = getPriv();

    auto globalSymbolUrl = priv->protocolBlock->makeSymbolUrl(name);

    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(globalSymbolUrl, /* allowMissing= */ true));
    if (symbol == nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kMissingMethod,
            "missing global method {}", name);

    if (symbol->getSymbolType() != SymbolType::CALL)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid call symbol {}", symbol->getSymbolUrl().toString());
    auto *callSymbol = cast_symbol_to_call(symbol);

    if (callSymbol->isHidden()) {
        if (!thisReceiver)
            return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                "cannot access hidden method {} on {}", name, m_protocolUrl.toString());
    }

    if (callSymbol->isBound())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid call symbol {}", callSymbol->getSymbolUrl().toString());

    auto callable = std::make_unique<FunctionCallable>(callSymbol, callSymbol->isInline());
    return invoker.initialize(std::move(callable));
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
