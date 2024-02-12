#ifndef LYRIC_BUILD_INTERNAL_TASK_UTILS_H
#define LYRIC_BUILD_INTERNAL_TASK_UTILS_H

#include <filesystem>

#include <lyric_common/assembly_location.h>
#include <tempo_utils/result.h>
#include <tempo_utils/url.h>

namespace lyric_build::internal {

    tempo_utils::Result<lyric_common::AssemblyLocation> convert_source_url_to_assembly_location(
        const tempo_utils::Url &sourceUrl,
        const tempo_utils::Url &baseUrl);

    std::filesystem::path generate_install_path(
        std::string_view taskDomain,
        const tempo_utils::Url &url,
        const char *dotSuffix = nullptr);

}

#endif // LYRIC_BUILD_INTERNAL_TASK_UTILS_H
