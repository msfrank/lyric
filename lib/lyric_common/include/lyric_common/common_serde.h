#ifndef LYRIC_COMMON_COMMON_SERDE_H
#define LYRIC_COMMON_COMMON_SERDE_H

#include <tempo_utils/attr.h>

#include "module_location.h"
#include "symbol_path.h"
#include "symbol_url.h"

namespace lyric_common {

    class ModuleLocationAttr : public tempo_utils::AttrSerde<ModuleLocation> {

        using SerdeType = ModuleLocation;

    public:
        ModuleLocationAttr(const tempo_utils::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_utils::AbstractAttrWriter *writer,
            const ModuleLocation &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_utils::AbstractAttrParser *parser,
            ModuleLocation &value) const override;
    };

    class SymbolPathAttr : public tempo_utils::AttrSerde<SymbolPath> {

        using SerdeType = SymbolPath;

    public:
        SymbolPathAttr(const tempo_utils::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_utils::AbstractAttrWriter *writer,
            const SymbolPath &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_utils::AbstractAttrParser *parser,
            SymbolPath &value) const override;
    };
}

#endif // LYRIC_COMMON_COMMON_SERDE_H