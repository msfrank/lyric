
#include <lyric_assembler/assembler_attrs.h>
#include <lyric_schema/assembler_schema.h>
#include <tempo_schema/schema_result.h>

lyric_assembler::OpcodeAttr::OpcodeAttr(const tempo_schema::ComparableResource *resource)
    : tempo_schema::AttrSerde<lyric_object::Opcode>(resource)
{
}

tempo_utils::Result<tu_uint32>
lyric_assembler::OpcodeAttr::writeAttr(tempo_schema::AbstractAttrWriter *writer, const lyric_object::Opcode &opcode) const
{
    TU_ASSERT (writer != nullptr);
    return writer->putUInt32(static_cast<tu_uint32>(opcode));
}

static tempo_utils::Status
value_to_opcode(tu_int64 value, lyric_object::Opcode &opcode)
{
    opcode = static_cast<lyric_object::Opcode>(value);
    if (lyric_object::opcode_to_name(opcode) == nullptr)
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kConversionError, "invalid opcode");
    return {};
}

tempo_utils::Status
lyric_assembler::OpcodeAttr::parseAttr(tu_uint32 index, tempo_schema::AbstractAttrParser *parser, lyric_object::Opcode &opcode) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    return value_to_opcode(value, opcode);
}

const lyric_assembler::OpcodeAttr lyric_assembler::kLyricAssemblerOpcodeEnum(
    &lyric_schema::kLyricAssemblerOpcodeEnumProperty);

const lyric_common::SymbolPathAttr lyric_assembler::kLyricAssemblerDefinitionSymbolPath(
    &lyric_schema::kLyricAssemblerDefinitionSymbolPathProperty);

const tempo_schema::StringAttr lyric_assembler::kLyricAssemblerTrapName(
    &lyric_schema::kLyricAssemblerTrapNameProperty);

const tempo_schema::UInt16Attr lyric_assembler::kLyricAssemblerStackOffset(
    &lyric_schema::kLyricAssemblerStackOffsetProperty);
