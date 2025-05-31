#ifndef LYRIC_ASSEMBLER_ASSEMBLER_ATTRS_H
#define LYRIC_ASSEMBLER_ASSEMBLER_ATTRS_H

#include <lyric_common/common_serde.h>
#include <tempo_schema/attr_serde.h>

namespace lyric_assembler {

    extern const lyric_common::SymbolPathAttr kLyricAssemblerDefinitionSymbolPath;
    extern const tempo_schema::StringAttr kLyricAssemblerTrapName;
}

#endif // LYRIC_ASSEMBLER_ASSEMBLER_ATTRS_H
