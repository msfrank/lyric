
#include <lyric_build/build_attrs.h>
#include <tempo_schema/schema_result.h>

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

const tempo_schema::PathAttr lyric_build::kLyricBuildRuntimeLibDirectory(
    &lyric_schema::kLyricBuildRuntimeLibDirectoryProperty);

const tempo_schema::PathAttr lyric_build::kLyricBuildLibDirectory(
    &lyric_schema::kLyricBuildLibDirectoryProperty);
