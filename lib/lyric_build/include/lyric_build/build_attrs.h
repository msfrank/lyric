#ifndef LYRIC_BUILD_BUILD_ATTRS_H
#define LYRIC_BUILD_BUILD_ATTRS_H

#include <lyric_build/build_types.h>
#include <lyric_common/common_serde.h>
#include <lyric_schema/build_schema.h>
#include <tempo_schema/url_serde.h>

namespace lyric_build {

    class EntryTypeAttr : public tempo_schema::AttrSerde<EntryType> {

        using SerdeType = EntryType;

    public:
        explicit EntryTypeAttr(const tempo_schema::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_schema::AbstractAttrWriter *writer,
            const EntryType &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_schema::AbstractAttrParser *parser,
            EntryType &value) const override;
    };

    extern const lyric_common::ModuleLocationAttr kLyricBuildModuleLocation;
    extern const tempo_schema::UrlAttr kLyricBuildContentUrl;
    extern const tempo_schema::StringAttr kLyricBuildContentType;
    extern const EntryTypeAttr kLyricBuildEntryType;
    extern const tempo_schema::StringAttr kLyricBuildGeneration;
    extern const tempo_schema::StringAttr kLyricBuildInstallPath;
    extern const tempo_schema::StringAttr kLyricBuildTaskHash;
    extern const tempo_schema::StringAttr kLyricBuildTaskParams;
};

#endif // LYRIC_BUILD_BUILD_ATTRS_H
