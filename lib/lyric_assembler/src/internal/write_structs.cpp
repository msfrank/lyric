
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

    TU_RETURN_IF_NOT_OK (writer.touchConstructor(structSymbol->getCtor()));

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
    std::vector<flatbuffers::Offset<lyo1::StructDescriptor>> &structs_vector,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    auto index = static_cast<tu_uint32>(structs_vector.size());

    auto classPathString = structSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fullyQualifiedName = buffer.CreateSharedString(classPathString);

    tu_uint32 structType;
    TU_ASSIGN_OR_RETURN (structType, writer.getTypeOffset(structSymbol->structType()->getTypeDef()));

    auto superstructIndex = lyric_runtime::INVALID_ADDRESS_U32;
    auto *superstructSymbol = structSymbol->superStruct();
    if (superstructSymbol != nullptr) {
        TU_ASSIGN_OR_RETURN (superstructIndex,
            writer.getSymbolAddress(superstructSymbol->getSymbolUrl(), lyric_object::LinkageSection::Struct));
    }

    lyo1::StructFlags structFlags = lyo1::StructFlags::NONE;
    if (structSymbol->isDeclOnly())
        structFlags |= lyo1::StructFlags::DeclOnly;
    if (structSymbol->isAbstract())
        structFlags |= lyo1::StructFlags::Abstract;
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
    switch (structSymbol->getAccessType()) {
        case lyric_object::AccessType::Public:
            structFlags |= lyo1::StructFlags::GlobalVisibility;
            break;
        case lyric_object::AccessType::Protected:
            structFlags |= lyo1::StructFlags::InheritVisibility;
            break;
        case lyric_object::AccessType::Private:
            break;
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                "invalid struct access");
    }

    // serialize array of members
    std::vector<tu_uint32> members;
    for (auto iterator = structSymbol->membersBegin(); iterator != structSymbol->membersEnd(); iterator++) {
        const auto &memberRef = iterator->second;
        tu_uint32 fieldIndex;
        TU_ASSIGN_OR_RETURN (fieldIndex,
            writer.getSymbolAddress(memberRef.symbolUrl, lyric_object::LinkageSection::Field));
        members.push_back(fieldIndex);
    }

    // serialize array of methods
    std::vector<tu_uint32> methods;
    for (auto iterator = structSymbol->methodsBegin(); iterator != structSymbol->methodsEnd(); iterator++) {
        const auto &boundMethod = iterator->second;
        tu_uint32 callIndex;
        TU_ASSIGN_OR_RETURN (callIndex,
            writer.getSymbolAddress(boundMethod.methodCall, lyric_object::LinkageSection::Call));
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
    tu_uint32 ctorCall;
    TU_ASSIGN_OR_RETURN (ctorCall,
        writer.getSymbolAddress(structSymbol->getCtor(), lyric_object::LinkageSection::Call));

    // serialize the sealed subtypes
    std::vector<tu_uint32> sealedSubtypes;
    for (auto iterator = structSymbol->sealedTypesBegin(); iterator != structSymbol->sealedTypesEnd(); iterator++) {
        tu_uint32 sealedSubtype;
        TU_ASSIGN_OR_RETURN (sealedSubtype, writer.getTypeOffset(*iterator));
        sealedSubtypes.push_back(sealedSubtype);
    }

    // add struct descriptor
    structs_vector.push_back(lyo1::CreateStructDescriptor(buffer, fullyQualifiedName,
        superstructIndex, structType, structFlags,
        buffer.CreateVector(members), buffer.CreateVector(methods), buffer.CreateVector(impls),
        structSymbol->getAllocatorTrap(), ctorCall, buffer.CreateVector(sealedSubtypes)));

    // add symbol descriptor
    symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, fullyQualifiedName,
        lyo1::DescriptorSection::Struct, index));

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_structs(
    const std::vector<const StructSymbol *> &structs,
    const ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    StructsOffset &structsOffset,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    std::vector<flatbuffers::Offset<lyo1::StructDescriptor>> structs_vector;

    for (const auto *structSymbol : structs) {
        TU_RETURN_IF_NOT_OK (write_struct(structSymbol, writer, buffer, structs_vector, symbols_vector));
    }

    // create the structs vector
    structsOffset = buffer.CreateVector(structs_vector);

    return {};
}
