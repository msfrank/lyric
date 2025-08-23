
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/internal/write_enums.h>
#include <lyric_assembler/object_writer.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

tempo_utils::Status
lyric_assembler::internal::touch_enum(
    const EnumSymbol *enumSymbol,
    const ObjectState *objectState,
    ObjectWriter &writer)
{
    TU_ASSERT (enumSymbol != nullptr);

    auto enumUrl = enumSymbol->getSymbolUrl();

    bool alreadyInserted;
    TU_RETURN_IF_NOT_OK (writer.insertSymbol(enumUrl, enumSymbol, alreadyInserted));
    if (alreadyInserted)
        return {};

    // if enum is an imported symbol then we are done
    if (enumSymbol->isImported())
        return {};

    TU_RETURN_IF_NOT_OK (writer.touchType(enumSymbol->enumType()));

    TU_RETURN_IF_NOT_OK (writer.touchConstructor(enumSymbol->getCtor()));

    for (auto it = enumSymbol->membersBegin(); it != enumSymbol->membersEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchMember(it->second));
    }

    for (auto it = enumSymbol->methodsBegin(); it != enumSymbol->methodsEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchMethod(it->second));
    }

    for (auto it = enumSymbol->implsBegin(); it != enumSymbol->implsEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchImpl(it->second));
    }

    for (auto it = enumSymbol->sealedTypesBegin(); it != enumSymbol->sealedTypesEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchType(*it));
    }

    return {};
}

static tempo_utils::Status
write_enum(
    const lyric_assembler::EnumSymbol *enumSymbol,
    const lyric_assembler::ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::EnumDescriptor>> &enums_vector)
{
    auto enumPathString = enumSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fullyQualifiedName = buffer.CreateSharedString(enumPathString);

    tu_uint32 enumType;
    TU_ASSIGN_OR_RETURN (enumType, writer.getTypeOffset(enumSymbol->enumType()->getTypeDef()));

    auto superenumIndex = lyric_runtime::INVALID_ADDRESS_U32;
    auto *superenumSymbol = enumSymbol->superEnum();
    if (superenumSymbol != nullptr) {
        TU_ASSIGN_OR_RETURN (superenumIndex,
            writer.getSectionAddress(superenumSymbol->getSymbolUrl(), lyric_object::LinkageSection::Enum));
    }

    lyo1::EnumFlags enumFlags = lyo1::EnumFlags::NONE;
    if (enumSymbol->isDeclOnly()) {
        enumFlags |= lyo1::EnumFlags::DeclOnly;
    }
    if (enumSymbol->isHidden()) {
        enumFlags |= lyo1::EnumFlags::Hidden;
    }

    switch (enumSymbol->getDeriveType()) {
        case lyric_object::DeriveType::Final:
            enumFlags |= lyo1::EnumFlags::Final;
            break;
        case lyric_object::DeriveType::Sealed:
            enumFlags |= lyo1::EnumFlags::Sealed;
            break;
        default:
            break;
    }

    // serialize array of members
    std::vector<tu_uint32> members;
    for (auto iterator = enumSymbol->membersBegin(); iterator != enumSymbol->membersEnd(); iterator++) {
        const auto &memberRef = iterator->second;
        tu_uint32 fieldIndex;
        TU_ASSIGN_OR_RETURN (fieldIndex,
            writer.getSectionAddress(memberRef.symbolUrl, lyric_object::LinkageSection::Field));
        members.push_back(fieldIndex);
    }

    // serialize array of methods
    std::vector<tu_uint32> methods;
    for (auto iterator = enumSymbol->methodsBegin(); iterator != enumSymbol->methodsEnd(); iterator++) {
        const auto &boundMethod = iterator->second;
        tu_uint32 callIndex;
        TU_ASSIGN_OR_RETURN (callIndex,
            writer.getSectionAddress(boundMethod.methodCall, lyric_object::LinkageSection::Call));
        methods.push_back(callIndex);
    }

    // serialize array of impls
    std::vector<tu_uint32> impls;
    for (auto iterator = enumSymbol->implsBegin(); iterator != enumSymbol->implsEnd(); iterator++) {
        auto *implHandle = iterator->second;
        tu_uint32 implIndex;
        TU_ASSIGN_OR_RETURN (implIndex, writer.getImplOffset(implHandle->getRef()));
        impls.push_back(implIndex);
    }

    // get enum ctor
    tu_uint32 ctorCall;
    TU_ASSIGN_OR_RETURN (ctorCall,
        writer.getSectionAddress(enumSymbol->getCtor(), lyric_object::LinkageSection::Call));

    // serialize the sealed subtypes
    std::vector<tu_uint32> sealedSubtypes;
    for (auto iterator = enumSymbol->sealedTypesBegin(); iterator != enumSymbol->sealedTypesEnd(); iterator++) {
        tu_uint32 sealedSubtype;
        TU_ASSIGN_OR_RETURN (sealedSubtype, writer.getTypeOffset(*iterator));
        sealedSubtypes.push_back(sealedSubtype);
    }

    tu_uint32 allocatorTrap = lyric_object::INVALID_ADDRESS_U32;
    auto trapName = enumSymbol->getAllocatorTrap();
    if (!trapName.empty()) {
        TU_ASSIGN_OR_RETURN (allocatorTrap, writer.getTrapNumber(trapName));
    }

    // add enum descriptor
    enums_vector.push_back(lyo1::CreateEnumDescriptor(buffer, fullyQualifiedName,
        superenumIndex, enumType, enumFlags,
        buffer.CreateVector(members), buffer.CreateVector(methods), buffer.CreateVector(impls),
        allocatorTrap, ctorCall, buffer.CreateVector(sealedSubtypes)));

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_enums(
    const std::vector<const EnumSymbol *> &enums,
    const ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    EnumsOffset &enumsOffset)
{
    std::vector<flatbuffers::Offset<lyo1::EnumDescriptor>> enums_vector;

    for (const auto *enumSymbol :enums) {
        TU_RETURN_IF_NOT_OK (write_enum(enumSymbol, writer, buffer, enums_vector));
    }

    // create the enums vector
    enumsOffset = buffer.CreateVector(enums_vector);

    return {};
}
