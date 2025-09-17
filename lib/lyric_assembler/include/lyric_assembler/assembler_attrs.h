#ifndef LYRIC_ASSEMBLER_ASSEMBLER_ATTRS_H
#define LYRIC_ASSEMBLER_ASSEMBLER_ATTRS_H

#include <lyric_common/common_serde.h>
#include <lyric_object/object_types.h>
#include <tempo_schema/attr_serde.h>

namespace lyric_assembler {

    class OpcodeAttr : public tempo_schema::AttrSerde<lyric_object::Opcode> {

        using SerdeType = lyric_object::Opcode;

    public:
        explicit OpcodeAttr(const tempo_schema::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_schema::AbstractAttrWriter *writer,
            const lyric_object::Opcode &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_schema::AbstractAttrParser *parser,
            lyric_object::Opcode &value) const override;
    };

    extern const OpcodeAttr kLyricAssemblerOpcodeEnum;
    extern const lyric_common::SymbolPathAttr kLyricAssemblerDefinitionSymbolPath;
    extern const tempo_schema::StringAttr kLyricAssemblerTrapName;
    extern const tempo_schema::UInt16Attr kLyricAssemblerStackOffset;
}

#endif // LYRIC_ASSEMBLER_ASSEMBLER_ATTRS_H
