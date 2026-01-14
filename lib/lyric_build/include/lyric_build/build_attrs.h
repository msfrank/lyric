#ifndef LYRIC_BUILD_BUILD_ATTRS_H
#define LYRIC_BUILD_BUILD_ATTRS_H

#include <lyric_build/build_types.h>
#include <lyric_common/common_serde.h>
#include <lyric_schema/build_schema.h>
#include <tempo_schema/path_serde.h>
#include <tempo_schema/url_serde.h>

namespace lyric_build {

    extern const lyric_common::ModuleLocationAttr kLyricBuildModuleLocation;
    extern const tempo_schema::UrlAttr kLyricBuildContentUrl;
    extern const tempo_schema::StringAttr kLyricBuildContentType;
    extern const tempo_schema::StringAttr kLyricBuildGeneration;
    extern const tempo_schema::StringAttr kLyricBuildInstallPath;
    extern const tempo_schema::StringAttr kLyricBuildTaskHash;
    extern const tempo_schema::StringAttr kLyricBuildTaskParams;
    extern const tempo_schema::PathAttr kLyricBuildRuntimeLibDirectory;
    extern const tempo_schema::PathAttr kLyricBuildLibDirectory;
};

#endif // LYRIC_BUILD_BUILD_ATTRS_H
