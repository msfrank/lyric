#ifndef LYRIC_BUILD_INTERNAL_TASK_UTILS_H
#define LYRIC_BUILD_INTERNAL_TASK_UTILS_H

#include <filesystem>

#include <lyric_common/module_location.h>
#include <tempo_utils/result.h>
#include <tempo_utils/url.h>

namespace lyric_build::internal {

    tempo_utils::UrlPath build_full_path(
        const tempo_utils::UrlPath &path,
        const tempo_utils::UrlPath &base);

    std::filesystem::path to_absolute_path_within_base(
        const std::filesystem::path &baseDirectory,
        const tempo_utils::UrlPath &path);

    tempo_utils::Result<lyric_common::ModuleLocation> convert_source_path_to_module_location(
        const tempo_utils::UrlPath &path);

    tempo_utils::Result<tempo_utils::UrlPath> convert_module_location_to_artifact_path(
        const lyric_common::ModuleLocation &location,
        std::string_view dotSuffix);

    std::filesystem::path generate_install_path(
        std::string_view taskDomain,
        const tempo_utils::UrlPath &path,
        const char *dotSuffix = nullptr);

}

#endif // LYRIC_BUILD_INTERNAL_TASK_UTILS_H
