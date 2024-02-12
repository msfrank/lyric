
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/internal/write_fields.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

static lyric_assembler::AssemblerStatus
write_field(
    lyric_assembler::FieldSymbol *fieldSymbol,
    lyric_assembler::TypeCache *typeCache,
    lyric_assembler::SymbolCache *symbolCache,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::FieldDescriptor>> &fields_vector,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    auto index = static_cast<uint32_t>(fields_vector.size());

    auto fieldPathString = fieldSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fullyQualifiedName = buffer.CreateSharedString(fieldPathString);
    auto fieldType = fieldSymbol->getAssignableType();
    auto *typeHandle = typeCache->getType(fieldType);

    lyo1::FieldFlags fieldFlags = lyo1::FieldFlags::NONE;
    if (fieldSymbol->isVariable())
        fieldFlags |= lyo1::FieldFlags::Var;
    if (!fieldSymbol->getAddress().isValid())
        fieldFlags |= lyo1::FieldFlags::DeclOnly;
    switch (fieldSymbol->getAccessType()) {
        case lyric_object::AccessType::Public:
            fieldFlags |= lyo1::FieldFlags::GlobalVisibility;
            break;
        case lyric_object::AccessType::Protected:
            fieldFlags |= lyo1::FieldFlags::InheritVisibility;
            break;
        case lyric_object::AccessType::Private:
            break;
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                "invalid field access");
    }

    // add field descriptor
    fields_vector.push_back(lyo1::CreateFieldDescriptor(buffer, fullyQualifiedName,
        typeHandle->getAddress().getAddress(), fieldFlags));

    // add symbol descriptor
    symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, fullyQualifiedName,
        lyo1::DescriptorSection::Field, index));

    return lyric_assembler::AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::internal::write_fields(
    const AssemblyState *assemblyState,
    flatbuffers::FlatBufferBuilder &buffer,
    FieldsOffset &fieldsOffset,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    TU_ASSERT (assemblyState != nullptr);

    SymbolCache *symbolCache = assemblyState->symbolCache();
    TypeCache *typeCache = assemblyState->typeCache();
    std::vector<flatbuffers::Offset<lyo1::FieldDescriptor>> fields_vector;

    for (auto iterator = assemblyState->fieldsBegin(); iterator != assemblyState->fieldsEnd(); iterator++) {
        auto &fieldSymbol = *iterator;
        TU_RETURN_IF_NOT_OK (
            write_field(fieldSymbol, typeCache, symbolCache, buffer, fields_vector, symbols_vector));
    }

    // create the fields vector
    fieldsOffset = buffer.CreateVector(fields_vector);

    return AssemblerStatus::ok();
}
