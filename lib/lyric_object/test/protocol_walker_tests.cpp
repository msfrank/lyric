#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>
#include <lyric_object/lyric_object.h>

class ProtocolWalker : public ::testing::Test {
protected:
    lyric_object::LyricObject object;
    void SetUp() override {
        flatbuffers::FlatBufferBuilder buffer;
        std::vector<flatbuffers::Offset<lyo1::ProtocolDescriptor>> protocols_vector;
        std::vector<flatbuffers::Offset<lyo1::StructDescriptor>> structs_vector;
        std::vector<flatbuffers::Offset<lyo1::TypeDescriptor>> types_vector;

        structs_vector.push_back(lyo1::CreateStructDescriptor(
            buffer,
            buffer.CreateString("SendMessage")));

        auto sendAssignable = lyo1::CreateConcreteAssignable(buffer,
            lyo1::TypeSection::Struct,
            0,
            0);
        types_vector.push_back(lyo1::CreateTypeDescriptor(buffer,
            lyo1::Assignable::ConcreteAssignable,
            sendAssignable.Union(),
            lyric_object::INVALID_ADDRESS_U32));

        structs_vector.push_back(lyo1::CreateStructDescriptor(
            buffer,
            buffer.CreateString("RecvMessage")));

        auto receiveAssignable = lyo1::CreateConcreteAssignable(buffer,
            lyo1::TypeSection::Struct,
            1,
            0);
        types_vector.push_back(lyo1::CreateTypeDescriptor(buffer,
            lyo1::Assignable::ConcreteAssignable,
            receiveAssignable.Union(),
            lyric_object::INVALID_ADDRESS_U32));

        protocols_vector.push_back(lyo1::CreateProtocolDescriptor(
            buffer,
            buffer.CreateString("Protocol"),
            lyo1::PortType::Connect,
            lyo1::CommunicationType::SendAndReceive,
            0,
            1));

        auto fb_protocols = buffer.CreateVector(protocols_vector);
        auto fb_structs = buffer.CreateVector(structs_vector);
        auto fb_types = buffer.CreateVector(types_vector);

        lyo1::ObjectBuilder objectBuilder(buffer);
        objectBuilder.add_protocols(fb_protocols);
        objectBuilder.add_structs(fb_structs);
        objectBuilder.add_types(fb_types);
        auto root = objectBuilder.Finish();
        buffer.Finish(root, lyo1::ObjectIdentifier());

        std::span<const tu_uint8> bytes(buffer.GetBufferPointer(), buffer.GetSize());
        object = lyric_object::LyricObject(bytes);
        TU_ASSERT (object.isValid());
        TU_ASSERT (lyric_object::LyricObject::verify(bytes));
    }
};

TEST_F(ProtocolWalker, ParseProtocol)
{
    ASSERT_EQ (1, object.numProtocols());

    auto protocolWalker = object.getProtocol(0);
    ASSERT_TRUE (protocolWalker.isValid());

    ASSERT_EQ (lyric_object::PortType::Connect, protocolWalker.getPort());
    ASSERT_EQ (lyric_object::CommunicationType::SendAndReceive, protocolWalker.getCommunication());

    auto sendType = protocolWalker.getSendType();
    auto sendTypeDef = sendType.getTypeDef();
    ASSERT_EQ (lyric_common::TypeDefType::Concrete, sendTypeDef.getType());
    ASSERT_EQ (lyric_common::SymbolUrl(lyric_common::SymbolPath({"SendMessage"})), sendTypeDef.getConcreteUrl());

    auto recvType = protocolWalker.getReceiveType();
    auto recvTypeDef = recvType.getTypeDef();
    ASSERT_EQ (lyric_common::TypeDefType::Concrete, recvTypeDef.getType());
    ASSERT_EQ (lyric_common::SymbolUrl(lyric_common::SymbolPath({"RecvMessage"})), recvTypeDef.getConcreteUrl());
}
