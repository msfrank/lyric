#ifndef LYRIC_COMMON_COMMON_SERDE_H
#define LYRIC_COMMON_COMMON_SERDE_H

#include <tempo_utils/attr.h>

#include "assembly_location.h"
#include "symbol_path.h"
#include "symbol_url.h"

namespace lyric_common {

    class AssemblyLocationAttr : public tempo_utils::AttrSerde<AssemblyLocation> {

        using SerdeType = AssemblyLocation;

    public:
        AssemblyLocationAttr(const tempo_utils::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_utils::AbstractAttrWriter *writer,
            const AssemblyLocation &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_utils::AbstractAttrParser *parser,
            AssemblyLocation &value) const override;
        tempo_utils::Status validateAttr(tu_uint32 index, tempo_utils::AbstractAttrParser *parser) const override;
        std::string toString(tu_uint32 index, tempo_utils::AbstractAttrParser *parser) const override;
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
        tempo_utils::Status validateAttr(tu_uint32 index, tempo_utils::AbstractAttrParser *parser) const override;
        std::string toString(tu_uint32 index, tempo_utils::AbstractAttrParser *parser) const override;
    };
}

#endif // LYRIC_COMMON_COMMON_SERDE_H