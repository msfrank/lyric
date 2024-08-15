
#include <lyric_common/common_serde.h>

lyric_common::ModuleLocationAttr::ModuleLocationAttr(const tempo_utils::ComparableResource *resource)
    : AttrSerde<ModuleLocation>(resource)
{
}

tempo_utils::Result<tu_uint32>
lyric_common::ModuleLocationAttr::writeAttr(
    tempo_utils::AbstractAttrWriter *writer,
    const ModuleLocation &location) const
{
    TU_ASSERT (writer != nullptr);
    return writer->putString(location.toString());
}

tempo_utils::Status
lyric_common::ModuleLocationAttr::parseAttr(
    tu_uint32 index,
    tempo_utils::AbstractAttrParser *parser,
    ModuleLocation &location) const
{
    std::string value;
    auto status = parser->getString(index, value);
    if (status.notOk())
        return status;
    location = ModuleLocation::fromString(value);
    return tempo_utils::AttrStatus::ok();
}

lyric_common::SymbolPathAttr::SymbolPathAttr(const tempo_utils::ComparableResource *resource)
    : AttrSerde<SymbolPath>(resource)
{
}

tempo_utils::Result<tu_uint32>
lyric_common::SymbolPathAttr::writeAttr(
    tempo_utils::AbstractAttrWriter *writer,
    const SymbolPath &location) const
{
    TU_ASSERT (writer != nullptr);
    return writer->putString(location.toString());
}

tempo_utils::Status
lyric_common::SymbolPathAttr::parseAttr(
    tu_uint32 index,
    tempo_utils::AbstractAttrParser *parser,
    SymbolPath &location) const
{
    std::string value;
    auto status = parser->getString(index, value);
    if (status.notOk())
        return status;
    location = SymbolPath::fromString(value);
    return tempo_utils::AttrStatus::ok();
}