
#include <lyric_build/build_attrs.h>
#include <tempo_schema/schema_result.h>

// lyric_build::EntryTypeAttr::EntryTypeAttr(const tempo_schema::ComparableResource *resource)
//     : tempo_schema::AttrSerde<EntryType>(resource)
// {
// }
//
// tempo_utils::Result<tu_uint32>
// lyric_build::EntryTypeAttr::writeAttr(tempo_schema::AbstractAttrWriter *writer, const EntryType &entryType) const
// {
//     TU_ASSERT (writer != nullptr);
//     return writer->putUInt32(static_cast<tu_uint32>(entryType));
// }
//
// static tempo_schema::SchemaStatus
// value_to_entry_type(tu_uint32 value, lyric_build::EntryType &entryType)
// {
//     switch (static_cast<lyric_build::EntryType>(value)) {
//         case lyric_build::EntryType::File:
//             entryType = lyric_build::EntryType::File;
//             return {};
//         case lyric_build::EntryType::Link:
//             entryType = lyric_build::EntryType::Link;
//             return {};
//         case lyric_build::EntryType::Directory:
//             entryType = lyric_build::EntryType::Directory;
//             return {};
//         default:
//             return tempo_schema::SchemaStatus::forCondition(
//                 tempo_schema::SchemaCondition::kConversionError, "invalid entry type");
//     }
// }
//
// tempo_utils::Status
// lyric_build::EntryTypeAttr::parseAttr(tu_uint32 index, tempo_schema::AbstractAttrParser *parser, EntryType &binding) const
// {
//     tu_uint32 value;
//     auto status = parser->getUInt32(index, value);
//     if (status.notOk())
//         return status;
//     return value_to_entry_type(value, binding);
// }
//
// const lyric_build::EntryTypeAttr lyric_build::kLyricBuildEntryType(
//     &lyric_schema::kLyricBuildEntryEnumProperty);

const lyric_common::ModuleLocationAttr lyric_build::kLyricBuildModuleLocation(
    &lyric_schema::kLyricBuildModuleLocationProperty);

const tempo_schema::UrlAttr lyric_build::kLyricBuildContentUrl(
    &lyric_schema::kLyricBuildContentUrlProperty);

const tempo_schema::StringAttr lyric_build::kLyricBuildContentType(
    &lyric_schema::kLyricBuildContentTypeProperty);

const tempo_schema::StringAttr lyric_build::kLyricBuildGeneration(
    &lyric_schema::kLyricBuildGenerationProperty);

const tempo_schema::StringAttr lyric_build::kLyricBuildInstallPath(
    &lyric_schema::kLyricBuildInstallPathProperty);

const tempo_schema::StringAttr lyric_build::kLyricBuildTaskHash(
    &lyric_schema::kLyricBuildTaskHashProperty);

const tempo_schema::StringAttr lyric_build::kLyricBuildTaskParams(
    &lyric_schema::kLyricBuildTaskParamsProperty);
