
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/protocol_walker.h>
#include <lyric_object/type_walker.h>

lyric_object::ProtocolWalker::ProtocolWalker()
    : m_protocolOffset(INVALID_ADDRESS_U32)
{
}

lyric_object::ProtocolWalker::ProtocolWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 protocolOffset)
    : m_reader(reader),
      m_protocolOffset(protocolOffset)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_object::ProtocolWalker::ProtocolWalker(const ProtocolWalker &other)
    : m_reader(other.m_reader),
      m_protocolOffset(other.m_protocolOffset)
{
}

bool
lyric_object::ProtocolWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_protocolOffset < m_reader->numProtocols();
}

lyric_common::SymbolPath
lyric_object::ProtocolWalker::getSymbolPath() const
{
    if (!isValid())
        return {};
    auto *protocolDescriptor = m_reader->getProtocol(m_protocolOffset);
    if (protocolDescriptor == nullptr)
        return {};
    if (protocolDescriptor->fqsn() == nullptr)
        return {};
    return lyric_common::SymbolPath::fromString(protocolDescriptor->fqsn()->str());
}

bool
lyric_object::ProtocolWalker::isDeclOnly() const
{
    if (!isValid())
        return false;
    auto *protocolDescriptor = m_reader->getProtocol(m_protocolOffset);
    if (protocolDescriptor == nullptr)
        return false;
    return bool(protocolDescriptor->flags() & lyo1::ProtocolFlags::DeclOnly);
}

lyric_object::PortType
lyric_object::ProtocolWalker::getPort() const
{
    if (!isValid())
        return PortType::Invalid;
    auto *protocolDescriptor = m_reader->getProtocol(m_protocolOffset);
    if (protocolDescriptor == nullptr)
        return PortType::Invalid;

    switch (protocolDescriptor->protocol_port()) {
        case lyo1::PortType::Accept:
            return PortType::Accept;
        case lyo1::PortType::Connect:
            return PortType::Connect;
        default:
            return PortType::Invalid;
    }
}

lyric_object::CommunicationType
lyric_object::ProtocolWalker::getCommunication() const
{
    if (!isValid())
        return CommunicationType::Invalid;
    auto *protocolDescriptor = m_reader->getProtocol(m_protocolOffset);
    if (protocolDescriptor == nullptr)
        return CommunicationType::Invalid;

    switch (protocolDescriptor->protocol_comm()) {
        case lyo1::CommunicationType::Receive:
            return CommunicationType::Receive;
        case lyo1::CommunicationType::Send:
            return CommunicationType::Send;
        case lyo1::CommunicationType::SendAndReceive:
            return CommunicationType::SendAndReceive;
        default:
            return CommunicationType::Invalid;
    }
}

lyric_object::TypeWalker
lyric_object::ProtocolWalker::getSendType() const
{
    if (!isValid())
        return {};
    auto *protocolDescriptor = m_reader->getProtocol(m_protocolOffset);
    if (protocolDescriptor == nullptr)
        return {};
    return TypeWalker(m_reader, protocolDescriptor->send_type());
}

lyric_object::TypeWalker
lyric_object::ProtocolWalker::getReceiveType() const
{
    if (!isValid())
        return {};
    auto *protocolDescriptor = m_reader->getProtocol(m_protocolOffset);
    if (protocolDescriptor == nullptr)
        return {};
    return TypeWalker(m_reader, protocolDescriptor->receive_type());
}

lyric_object::TypeWalker
lyric_object::ProtocolWalker::getProtocolType() const
{
    if (!isValid())
        return {};
    auto *protocolDescriptor = m_reader->getProtocol(m_protocolOffset);
    if (protocolDescriptor == nullptr)
        return {};
    return TypeWalker(m_reader, protocolDescriptor->protocol_type());
}

tu_uint32
lyric_object::ProtocolWalker::getDescriptorOffset() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    return m_protocolOffset;
}
