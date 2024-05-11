
#include <lyric_build/build_attrs.h>

lyric_build::EntryTypeAttr::EntryTypeAttr(const tempo_utils::ComparableResource *resource)
    : tempo_utils::AttrSerde<EntryType>(resource)
{
}

tempo_utils::Result<tu_uint32>
lyric_build::EntryTypeAttr::writeAttr(tempo_utils::AbstractAttrWriter *writer, const EntryType &entryType) const
{
    TU_ASSERT (writer != nullptr);
    return writer->putUInt32(static_cast<tu_uint32>(entryType));
}

static tempo_utils::AttrStatus
value_to_base_type(tu_uint32 value, lyric_build::EntryType &entryType)
{
    switch (static_cast<lyric_build::EntryType>(value)) {
        case lyric_build::EntryType::File:
            entryType = lyric_build::EntryType::File;
            return tempo_utils::AttrStatus::ok();
        case lyric_build::EntryType::Link:
            entryType = lyric_build::EntryType::Link;
            return tempo_utils::AttrStatus::ok();
        case lyric_build::EntryType::Directory:
            entryType = lyric_build::EntryType::Directory;
            return tempo_utils::AttrStatus::ok();
        default:
            return tempo_utils::AttrStatus::forCondition(
                tempo_utils::AttrCondition::kConversionError, "invalid entry type");
    }
}

tempo_utils::Status
lyric_build::EntryTypeAttr::parseAttr(tu_uint32 index, tempo_utils::AbstractAttrParser *parser, EntryType &binding) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    return value_to_base_type(value, binding);
}

tempo_utils::Status
lyric_build::EntryTypeAttr::validateAttr(tu_uint32 index, tempo_utils::AbstractAttrParser *parser) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    EntryType base;
    return value_to_base_type(value, base);
}

std::string
lyric_build::EntryTypeAttr::toString(tu_uint32 index, tempo_utils::AbstractAttrParser *parser) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return "???";

    EntryType entryType;
    status = value_to_base_type(value, entryType);
    if (status.notOk())
        return "???";

    switch (entryType) {
        case EntryType::File:
            return "File";
        case EntryType::Link:
            return "Link";
        case EntryType::Directory:
            return "Directory";
        default:
            return "???";
    }
}

const lyric_common::AssemblyLocationAttr lyric_build::kLyricBuildAssemblyLocation(
    &lyric_schema::kLyricBuildAssemblyLocationProperty);

const tempo_utils::UrlAttr lyric_build::kLyricBuildContentUrl(
    &lyric_schema::kLyricBuildContentUrlProperty);

const lyric_build::EntryTypeAttr lyric_build::kLyricBuildEntryType(
    &lyric_schema::kLyricBuildEntryEnumProperty);

const tempo_utils::StringAttr lyric_build::kLyricBuildGeneration(
    &lyric_schema::kLyricBuildGenerationProperty);

const tempo_utils::StringAttr lyric_build::kLyricBuildInstallPath(
    &lyric_schema::kLyricBuildInstallPathProperty);

const tempo_utils::StringAttr lyric_build::kLyricBuildTaskHash(
    &lyric_schema::kLyricBuildTaskHashProperty);

const tempo_utils::StringAttr lyric_build::kLyricBuildTaskParams(
    &lyric_schema::kLyricBuildTaskParamsProperty);
