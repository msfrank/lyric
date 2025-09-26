
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/internal/write_structs.h>
#include <lyric_assembler/object_writer.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

tempo_utils::Status
lyric_assembler::internal::touch_struct(
    const StructSymbol *structSymbol,
    const ObjectState *objectState,
    ObjectWriter &writer)
{
    TU_ASSERT (structSymbol != nullptr);

    auto structUrl = structSymbol->getSymbolUrl();

    bool alreadyInserted;
    TU_RETURN_IF_NOT_OK (writer.insertSymbol(structUrl, structSymbol, alreadyInserted));
    if (alreadyInserted)
        return {};

    // if struct is an imported symbol then we are done
    if (structSymbol->isImported())
        return {};

    TU_RETURN_IF_NOT_OK (writer.touchType(structSymbol->structType()));

    for (auto it = structSymbol->membersBegin(); it != structSymbol->membersEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchMember(it->second));
    }

    for (auto it = structSymbol->methodsBegin(); it != structSymbol->methodsEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchMethod(it->second));
    }

    for (auto it = structSymbol->implsBegin(); it != structSymbol->implsEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchImpl(it->second));
    }

    for (auto it = structSymbol->sealedTypesBegin(); it != structSymbol->sealedTypesEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchType(*it));
    }

    return {};
}

static tempo_utils::Status
write_struct(
    const lyric_assembler::StructSymbol *structSymbol,
    const lyric_assembler::ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::StructDescriptor>> &structs_vector)
{
    auto structPathString = structSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fullyQualifiedName = buffer.CreateSharedString(structPathString);

    tu_uint32 structType;
    TU_ASSIGN_OR_RETURN (structType, writer.getTypeOffset(structSymbol->structType()->getTypeDef()));

    auto superstructIndex = lyric_runtime::INVALID_ADDRESS_U32;
    auto *superstructSymbol = structSymbol->superStruct();
    if (superstructSymbol != nullptr) {
        TU_ASSIGN_OR_RETURN (superstructIndex,
            writer.getSectionAddress(superstructSymbol->getSymbolUrl(), lyric_object::LinkageSection::Struct));
    }

    lyo1::StructFlags structFlags = lyo1::StructFlags::NONE;
    if (structSymbol->isDeclOnly()) {
        structFlags |= lyo1::StructFlags::DeclOnly;
    }
    if (structSymbol->isHidden()) {
        structFlags |= lyo1::StructFlags::Hidden;
    }

    switch (structSymbol->getDeriveType()) {
        case lyric_object::DeriveType::Final:
            structFlags |= lyo1::StructFlags::Final;
            break;
        case lyric_object::DeriveType::Sealed:
            structFlags |= lyo1::StructFlags::Sealed;
            break;
        default:
            break;
    }

    // serialize array of members
    std::vector<tu_uint32> members;
    for (auto iterator = structSymbol->membersBegin(); iterator != structSymbol->membersEnd(); iterator++) {
        const auto &memberRef = iterator->second;
        tu_uint32 fieldIndex;
        TU_ASSIGN_OR_RETURN (fieldIndex,
            writer.getSectionAddress(memberRef.symbolUrl, lyric_object::LinkageSection::Field));
        members.push_back(fieldIndex);
    }

    // serialize array of methods
    std::vector<tu_uint32> methods;
    for (auto iterator = structSymbol->methodsBegin(); iterator != structSymbol->methodsEnd(); iterator++) {
        const auto &boundMethod = iterator->second;
        tu_uint32 callIndex;
        TU_ASSIGN_OR_RETURN (callIndex,
            writer.getSectionAddress(boundMethod.methodCall, lyric_object::LinkageSection::Call));
        methods.push_back(callIndex);
    }

    // serialize array of impls
    std::vector<tu_uint32> impls;
    for (auto iterator = structSymbol->implsBegin(); iterator != structSymbol->implsEnd(); iterator++) {
        auto *implHandle = iterator->second;
        tu_uint32 implIndex;
        TU_ASSIGN_OR_RETURN (implIndex, writer.getImplOffset(implHandle->getRef()));
        impls.push_back(implIndex);
    }

    // get struct ctor
    tu_uint32 ctorCall = lyric_runtime::INVALID_ADDRESS_U32;

    // serialize the sealed subtypes
    std::vector<tu_uint32> sealedSubtypes;
    for (auto iterator = structSymbol->sealedTypesBegin(); iterator != structSymbol->sealedTypesEnd(); iterator++) {
        tu_uint32 sealedSubtype;
        TU_ASSIGN_OR_RETURN (sealedSubtype, writer.getTypeOffset(*iterator));
        sealedSubtypes.push_back(sealedSubtype);
    }

    tu_uint32 allocatorTrap = lyric_object::INVALID_ADDRESS_U32;
    auto trapName = structSymbol->getAllocatorTrap();
    if (!trapName.empty()) {
        TU_ASSIGN_OR_RETURN (allocatorTrap, writer.getTrapNumber(trapName));
    }

    // add struct descriptor
    structs_vector.push_back(lyo1::CreateStructDescriptor(buffer, fullyQualifiedName,
        superstructIndex, structType, structFlags,
        buffer.CreateVector(members), buffer.CreateVector(methods), buffer.CreateVector(impls),
        allocatorTrap, ctorCall, buffer.CreateVector(sealedSubtypes)));

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_structs(
    const std::vector<const StructSymbol *> &structs,
    const ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    StructsOffset &structsOffset)
{
    std::vector<flatbuffers::Offset<lyo1::StructDescriptor>> structs_vector;

    for (const auto *structSymbol : structs) {
        TU_RETURN_IF_NOT_OK (write_struct(structSymbol, writer, buffer, structs_vector));
    }

    // create the structs vector
    structsOffset = buffer.CreateVector(structs_vector);

    return {};
}
