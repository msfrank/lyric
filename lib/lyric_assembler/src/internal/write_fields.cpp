
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/internal/write_fields.h>
#include <lyric_assembler/object_writer.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

tempo_utils::Status
lyric_assembler::internal::touch_field(
    const FieldSymbol *fieldSymbol,
    const ObjectState *objectState,
    ObjectWriter &writer)
{
    TU_ASSERT (fieldSymbol != nullptr);

    auto fieldUrl = fieldSymbol->getSymbolUrl();

    bool alreadyInserted;
    TU_RETURN_IF_NOT_OK (writer.insertSymbol(fieldUrl, fieldSymbol, alreadyInserted));
    if (alreadyInserted)
        return {};

    // if field is an imported symbol then we are done
    if (fieldSymbol->isImported())
        return {};

    TU_RETURN_IF_NOT_OK (writer.touchType(fieldSymbol->getTypeDef()));

    auto initializerUrl = fieldSymbol->getInitializer();
    if (initializerUrl.isValid()) {
        TU_RETURN_IF_NOT_OK (writer.touchInitializer(initializerUrl));
    }

    return {};
}

static tempo_utils::Status
write_field(
    const lyric_assembler::FieldSymbol *fieldSymbol,
    const lyric_assembler::ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::FieldDescriptor>> &fields_vector,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    auto index = static_cast<tu_uint32>(fields_vector.size());

    auto fieldPathString = fieldSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fullyQualifiedName = buffer.CreateSharedString(fieldPathString);

    tu_uint32 fieldType;
    TU_ASSIGN_OR_RETURN (fieldType, writer.getTypeOffset(fieldSymbol->getTypeDef()));

    lyo1::FieldFlags fieldFlags = lyo1::FieldFlags::NONE;
    if (fieldSymbol->isVariable())
        fieldFlags |= lyo1::FieldFlags::Var;
    if (fieldSymbol->isDeclOnly())
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
    fields_vector.push_back(lyo1::CreateFieldDescriptor(buffer, fullyQualifiedName, fieldType, fieldFlags));

    // add symbol descriptor
    symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, fullyQualifiedName,
        lyo1::DescriptorSection::Field, index));

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_fields(
    const std::vector<const FieldSymbol *> &fields,
    const ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    FieldsOffset &fieldsOffset,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    std::vector<flatbuffers::Offset<lyo1::FieldDescriptor>> fields_vector;

    for (const auto *fieldSymbol : fields) {
        TU_RETURN_IF_NOT_OK (write_field(fieldSymbol, writer, buffer, fields_vector, symbols_vector));
    }

    // create the fields vector
    fieldsOffset = buffer.CreateVector(fields_vector);

    return {};
}
