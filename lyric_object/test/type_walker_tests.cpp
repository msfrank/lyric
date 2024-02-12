#include <gtest/gtest.h>

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>
#include <lyric_object/lyric_object.h>

TEST(TypeWalker, ParseConcreteType)
{
    flatbuffers::FlatBufferBuilder buffer;
    std::vector<flatbuffers::Offset<lyo1::ExistentialDescriptor>> existentials_vector;
    std::vector<flatbuffers::Offset<lyo1::TypeDescriptor>> types_vector;

    existentials_vector.push_back(lyo1::CreateExistentialDescriptor(
        buffer,
        buffer.CreateString("Test")));

    auto assignable = lyo1::CreateConcreteAssignable(buffer,
        lyo1::TypeSection::Existential,
        0,
        0);
    types_vector.push_back(lyo1::CreateTypeDescriptor(buffer,
        lyo1::Assignable::ConcreteAssignable,
        assignable.Union(),
        lyric_object::INVALID_ADDRESS_U32));

    auto fb_existentials = buffer.CreateVector(existentials_vector);
    auto fb_types = buffer.CreateVector(types_vector);

    lyo1::ObjectBuilder objectBuilder(buffer);
    objectBuilder.add_existentials(fb_existentials);
    objectBuilder.add_types(fb_types);
    auto root = objectBuilder.Finish();
    buffer.Finish(root, lyo1::ObjectIdentifier());

    std::span<const tu_uint8> bytes(buffer.GetBufferPointer(), buffer.GetSize());
    lyric_object::LyricObject object(bytes);
    ASSERT_TRUE (lyric_object::LyricObject::verify(bytes));

    auto objectWalker = object.getObject();
    ASSERT_TRUE (objectWalker.isValid());
    ASSERT_EQ (1, objectWalker.numTypes());

    auto typeWalker = objectWalker.getType(0);
    ASSERT_TRUE (typeWalker.isValid());
    ASSERT_EQ (lyric_common::TypeDefType::Concrete, typeWalker.getTypeDefType());

    auto typeDef = typeWalker.getTypeDef();
    ASSERT_EQ (lyric_common::TypeDefType::Concrete, typeDef.getType());
    ASSERT_EQ (lyric_common::SymbolUrl(lyric_common::SymbolPath({"Test"})), typeDef.getConcreteUrl());
}

TEST(TypeWalker, ParsePlaceholderType)
{
    flatbuffers::FlatBufferBuilder buffer;
    std::vector<flatbuffers::Offset<lyo1::ExistentialDescriptor>> existentials_vector;
    std::vector<flatbuffers::Offset<lyo1::TemplateDescriptor>> templates_vector;
    std::vector<flatbuffers::Offset<lyo1::TypeDescriptor>> types_vector;

    existentials_vector.push_back(lyo1::CreateExistentialDescriptor(
        buffer,
        buffer.CreateString("Test")));

    templates_vector.push_back(lyo1::CreateTemplateDescriptor(buffer,
        buffer.CreateString("Test"),
        0,
        0,
        0));

    auto assignable = lyo1::CreatePlaceholderAssignable(buffer,
        0,
        0,
        0);
    types_vector.push_back(lyo1::CreateTypeDescriptor(buffer,
        lyo1::Assignable::PlaceholderAssignable,
        assignable.Union(),
        lyric_object::INVALID_ADDRESS_U32));

    auto fb_existentials = buffer.CreateVector(existentials_vector);
    auto fb_templates = buffer.CreateVector(templates_vector);
    auto fb_types = buffer.CreateVector(types_vector);

    lyo1::ObjectBuilder objectBuilder(buffer);
    objectBuilder.add_existentials(fb_existentials);
    objectBuilder.add_templates(fb_templates);
    objectBuilder.add_types(fb_types);
    auto root = objectBuilder.Finish();
    buffer.Finish(root, lyo1::ObjectIdentifier());

    std::span<const tu_uint8> bytes(buffer.GetBufferPointer(), buffer.GetSize());
    lyric_object::LyricObject object(bytes);
    ASSERT_TRUE (lyric_object::LyricObject::verify(bytes));

    auto objectWalker = object.getObject();
    ASSERT_TRUE (objectWalker.isValid());
    ASSERT_EQ (1, objectWalker.numTypes());

    auto typeWalker = objectWalker.getType(0);
    ASSERT_TRUE (typeWalker.isValid());
    ASSERT_EQ (lyric_common::TypeDefType::Placeholder, typeWalker.getTypeDefType());

    auto typeDef = typeWalker.getTypeDef();
    ASSERT_EQ (lyric_common::TypeDefType::Placeholder, typeDef.getType());
    ASSERT_EQ (lyric_common::SymbolUrl(lyric_common::SymbolPath({"Test"})), typeDef.getPlaceholderTemplateUrl());
    ASSERT_EQ (0, typeDef.getPlaceholderIndex());
}

TEST(TypeWalker, ParseIntersectionType)
{
    flatbuffers::FlatBufferBuilder buffer;
    std::vector<flatbuffers::Offset<lyo1::ExistentialDescriptor>> existentials_vector;
    std::vector<flatbuffers::Offset<lyo1::TypeDescriptor>> types_vector;

    existentials_vector.push_back(lyo1::CreateExistentialDescriptor(
        buffer,
        buffer.CreateString("A"),
        lyric_object::INVALID_ADDRESS_U32,
        lyric_object::INVALID_ADDRESS_U32,
        0));
    auto concreteA = lyo1::CreateConcreteAssignable(buffer,
        lyo1::TypeSection::Existential, 0, 0);
    types_vector.push_back(lyo1::CreateTypeDescriptor(buffer,
        lyo1::Assignable::ConcreteAssignable,
        concreteA.Union(),
        lyric_object::INVALID_ADDRESS_U32));

    existentials_vector.push_back(lyo1::CreateExistentialDescriptor(
        buffer,
        buffer.CreateString("B"),
        lyric_object::INVALID_ADDRESS_U32,
        lyric_object::INVALID_ADDRESS_U32,
        1));
    auto concreteB = lyo1::CreateConcreteAssignable(buffer,
        lyo1::TypeSection::Existential, 1, 0);
    types_vector.push_back(lyo1::CreateTypeDescriptor(buffer,
        lyo1::Assignable::ConcreteAssignable,
        concreteB.Union(),
        lyric_object::INVALID_ADDRESS_U32));

    existentials_vector.push_back(lyo1::CreateExistentialDescriptor(
        buffer,
        buffer.CreateString("C"),
        lyric_object::INVALID_ADDRESS_U32,
        lyric_object::INVALID_ADDRESS_U32,
        2));
    auto concreteC = lyo1::CreateConcreteAssignable(buffer,
        lyo1::TypeSection::Existential, 2, 0);
    types_vector.push_back(lyo1::CreateTypeDescriptor(buffer,
        lyo1::Assignable::ConcreteAssignable,
        concreteC.Union(),
        lyric_object::INVALID_ADDRESS_U32));

    std::vector<tu_uint32> members = {0, 1, 2};
    auto intersection = lyo1::CreateIntersectionAssignable(buffer,
        buffer.CreateVector(members));

    types_vector.push_back(lyo1::CreateTypeDescriptor(buffer,
        lyo1::Assignable::IntersectionAssignable,
        intersection.Union(),
        lyric_object::INVALID_ADDRESS_U32));

    auto fb_existentials = buffer.CreateVector(existentials_vector);
    auto fb_types = buffer.CreateVector(types_vector);

    lyo1::ObjectBuilder objectBuilder(buffer);
    objectBuilder.add_existentials(fb_existentials);
    objectBuilder.add_types(fb_types);
    auto root = objectBuilder.Finish();
    buffer.Finish(root, lyo1::ObjectIdentifier());

    std::span<const tu_uint8> bytes(buffer.GetBufferPointer(), buffer.GetSize());
    lyric_object::LyricObject object(bytes);
    ASSERT_TRUE (lyric_object::LyricObject::verify(bytes));

    auto objectWalker = object.getObject();
    ASSERT_TRUE (objectWalker.isValid());
    ASSERT_EQ (4, objectWalker.numTypes());

    auto typeWalker = objectWalker.getType(3);
    ASSERT_TRUE (typeWalker.isValid());
    ASSERT_EQ (lyric_common::TypeDefType::Intersection, typeWalker.getTypeDefType());

    auto typeDef = typeWalker.getTypeDef();
    ASSERT_EQ (lyric_common::TypeDefType::Intersection, typeDef.getType());
    ASSERT_EQ (
        std::vector({
            lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl(lyric_common::SymbolPath({"A"}))),
            lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl(lyric_common::SymbolPath({"B"}))),
            lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl(lyric_common::SymbolPath({"C"}))),
        }),
        std::vector<lyric_common::TypeDef>(
            typeDef.intersectionMembersBegin(), typeDef.intersectionMembersEnd()));
}

TEST(TypeWalker, ParseUnionType)
{
    flatbuffers::FlatBufferBuilder buffer;
    std::vector<flatbuffers::Offset<lyo1::ExistentialDescriptor>> existentials_vector;
    std::vector<flatbuffers::Offset<lyo1::TypeDescriptor>> types_vector;

    existentials_vector.push_back(lyo1::CreateExistentialDescriptor(
        buffer,
        buffer.CreateString("A"),
        lyric_object::INVALID_ADDRESS_U32,
        lyric_object::INVALID_ADDRESS_U32,
        0));
    auto concreteA = lyo1::CreateConcreteAssignable(buffer,
        lyo1::TypeSection::Existential, 0, 0);
    types_vector.push_back(lyo1::CreateTypeDescriptor(buffer,
        lyo1::Assignable::ConcreteAssignable,
        concreteA.Union(),
        lyric_object::INVALID_ADDRESS_U32));

    existentials_vector.push_back(lyo1::CreateExistentialDescriptor(
        buffer,
        buffer.CreateString("B"),
        lyric_object::INVALID_ADDRESS_U32,
        lyric_object::INVALID_ADDRESS_U32,
        1));
    auto concreteB = lyo1::CreateConcreteAssignable(buffer,
        lyo1::TypeSection::Existential, 1, 0);
    types_vector.push_back(lyo1::CreateTypeDescriptor(buffer,
        lyo1::Assignable::ConcreteAssignable,
        concreteB.Union(),
        lyric_object::INVALID_ADDRESS_U32));

    existentials_vector.push_back(lyo1::CreateExistentialDescriptor(
        buffer,
        buffer.CreateString("C"),
        lyric_object::INVALID_ADDRESS_U32,
        lyric_object::INVALID_ADDRESS_U32,
        2));
    auto concreteC = lyo1::CreateConcreteAssignable(buffer,
        lyo1::TypeSection::Existential, 2, 0);
    types_vector.push_back(lyo1::CreateTypeDescriptor(buffer,
        lyo1::Assignable::ConcreteAssignable,
        concreteC.Union(),
        lyric_object::INVALID_ADDRESS_U32));

    std::vector<tu_uint32> members = {0, 1, 2};
    auto union_ = lyo1::CreateUnionAssignable(buffer,
        buffer.CreateVector(members));

    types_vector.push_back(lyo1::CreateTypeDescriptor(buffer,
        lyo1::Assignable::UnionAssignable,
        union_.Union(),
        lyric_object::INVALID_ADDRESS_U32));

    auto fb_existentials = buffer.CreateVector(existentials_vector);
    auto fb_types = buffer.CreateVector(types_vector);

    lyo1::ObjectBuilder objectBuilder(buffer);
    objectBuilder.add_existentials(fb_existentials);
    objectBuilder.add_types(fb_types);
    auto root = objectBuilder.Finish();
    buffer.Finish(root, lyo1::ObjectIdentifier());

    std::span<const tu_uint8> bytes(buffer.GetBufferPointer(), buffer.GetSize());
    lyric_object::LyricObject object(bytes);
    ASSERT_TRUE (lyric_object::LyricObject::verify(bytes));

    auto objectWalker = object.getObject();
    ASSERT_TRUE (objectWalker.isValid());
    ASSERT_EQ (4, objectWalker.numTypes());

    auto typeWalker = objectWalker.getType(3);
    ASSERT_TRUE (typeWalker.isValid());
    ASSERT_EQ (lyric_common::TypeDefType::Union, typeWalker.getTypeDefType());

    auto typeDef = typeWalker.getTypeDef();
    ASSERT_EQ (lyric_common::TypeDefType::Union, typeDef.getType());
    ASSERT_EQ (
        std::vector({
            lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl(lyric_common::SymbolPath({"A"}))),
            lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl(lyric_common::SymbolPath({"B"}))),
            lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl(lyric_common::SymbolPath({"C"}))),
        }),
        std::vector<lyric_common::TypeDef>(
            typeDef.unionMembersBegin(), typeDef.unionMembersEnd()));
}
