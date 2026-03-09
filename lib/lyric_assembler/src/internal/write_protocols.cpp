
#include <lyric_assembler/internal/write_protocols.h>
#include <lyric_assembler/object_writer.h>
#include <lyric_assembler/protocol_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

tempo_utils::Status
lyric_assembler::internal::touch_protocol(
    const ProtocolSymbol *protocolSymbol,
    const ObjectState *objectState,
    ObjectWriter &writer)
{
    TU_ASSERT (protocolSymbol != nullptr);

    auto protocolUrl = protocolSymbol->getSymbolUrl();

    bool alreadyInserted;
    TU_RETURN_IF_NOT_OK (writer.insertSymbol(protocolUrl, protocolSymbol, alreadyInserted));
    if (alreadyInserted)
        return {};

    // if protocol is an imported symbol then we are done
    if (protocolSymbol->isImported())
        return {};

    auto *sendType = protocolSymbol->sendType();
    TU_RETURN_IF_NOT_OK (writer.touchType(sendType->getTypeDef()));

    auto *receiveType = protocolSymbol->receiveType();
    TU_RETURN_IF_NOT_OK (writer.touchType(receiveType->getTypeDef()));

    return {};
}

static tempo_utils::Status
write_protocol(
    const lyric_assembler::ProtocolSymbol *protocolSymbol,
    const lyric_assembler::ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::ProtocolDescriptor>> &protocols_vector)
{
    auto protocolPathString = protocolSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fb_fullyQualifiedName = buffer.CreateSharedString(protocolPathString);

    lyo1::ProtocolFlags protocolFlags = lyo1::ProtocolFlags::NONE;

    if (protocolSymbol->isDeclOnly()) {
        protocolFlags |= lyo1::ProtocolFlags::DeclOnly;
    }

    lyo1::PortType port;
    switch (protocolSymbol->getPortType()) {
        case lyric_object::PortType::Accept:
            port = lyo1::PortType::Accept;
            break;
        case lyric_object::PortType::Connect:
            port = lyo1::PortType::Connect;
            break;
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid port type");
    }

    lyo1::CommunicationType comm;
    switch (protocolSymbol->getCommunicationType()) {
        case lyric_object::CommunicationType::Receive:
            comm = lyo1::CommunicationType::Receive;
            break;
        case lyric_object::CommunicationType::Send:
            comm = lyo1::CommunicationType::Send;
            break;
        case lyric_object::CommunicationType::SendAndReceive:
            comm = lyo1::CommunicationType::SendAndReceive;
            break;
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid communication type");
    }

    auto *sendType = protocolSymbol->sendType();
    tu_uint32 sendTypeOffset;
    TU_ASSIGN_OR_RETURN (sendTypeOffset, writer.getTypeOffset(sendType->getTypeDef()));

    auto *receiveType = protocolSymbol->receiveType();
    tu_uint32 receiveTypeOffset;
    TU_ASSIGN_OR_RETURN (receiveTypeOffset, writer.getTypeOffset(receiveType->getTypeDef()));

    // add protocol descriptor
    protocols_vector.push_back(lyo1::CreateProtocolDescriptor(buffer,
        fb_fullyQualifiedName, port, comm, sendTypeOffset, receiveTypeOffset, protocolFlags));

    return {};
}


tempo_utils::Status
lyric_assembler::internal::write_protocols(
    const std::vector<const ProtocolSymbol *> &protocols,
    const ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    ProtocolsOffset &protocolsOffset)
{
    std::vector<flatbuffers::Offset<lyo1::ProtocolDescriptor>> protocols_vector;

    for (const auto *protocolSymbol : protocols) {
        TU_RETURN_IF_NOT_OK (write_protocol(protocolSymbol, writer, buffer, protocols_vector));
    }

    // create the protocols vector
    protocolsOffset = buffer.CreateVector(protocols_vector);

    return {};
}
