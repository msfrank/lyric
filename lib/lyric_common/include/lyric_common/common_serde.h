#ifndef LYRIC_COMMON_COMMON_SERDE_H
#define LYRIC_COMMON_COMMON_SERDE_H

#include <tempo_schema/attr_serde.h>

#include "module_location.h"
#include "symbol_path.h"
#include "symbol_url.h"

namespace lyric_common {

    class ModuleLocationAttr : public tempo_schema::AttrSerde<ModuleLocation> {

        using SerdeType = ModuleLocation;

    public:
        ModuleLocationAttr(const tempo_schema::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_schema::AbstractAttrWriter *writer,
            const ModuleLocation &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_schema::AbstractAttrParser *parser,
            ModuleLocation &value) const override;
    };

    class SymbolPathAttr : public tempo_schema::AttrSerde<SymbolPath> {

        using SerdeType = SymbolPath;

    public:
        SymbolPathAttr(const tempo_schema::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_schema::AbstractAttrWriter *writer,
            const SymbolPath &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_schema::AbstractAttrParser *parser,
            SymbolPath &value) const override;
    };
}

#endif // LYRIC_COMMON_COMMON_SERDE_H