
#include <lyric_build/build_result.h>
#include <lyric_build/internal/task_utils.h>
#include <lyric_common/module_location.h>
#include <lyric_common/common_types.h>

tempo_utils::UrlPath
lyric_build::internal::build_full_path(const tempo_utils::UrlPath &path, const tempo_utils::UrlPath &base)
{
    if (!path.isValid())
        return {};
    if (path.isAbsolute())
        return path;
    if (base.isAbsolute())
        return base.traverse(path);
    auto absoluteBase = tempo_utils::UrlPath::fromString(absl::StrCat("/", base.toString()));
    return absoluteBase.traverse(path);
}

tempo_utils::Result<lyric_common::ModuleLocation>
lyric_build::internal::convert_source_path_to_module_location(const tempo_utils::UrlPath &path)
{
    std::filesystem::path p(path.toString(), std::filesystem::path::generic_format);
    if (p.extension() != std::string_view(lyric_common::kSourceFileDotSuffix))
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "{} is not a valid lyric source file", p.string());
    p.replace_extension();
    return lyric_common::ModuleLocation(p.string());
}

std::filesystem::path
lyric_build::internal::generate_install_path(
    std::string_view taskDomain,
    const tempo_utils::UrlPath &path,
    const char *dotSuffix)
{
    TU_ASSERT (!taskDomain.empty());
    TU_ASSERT (path.isValid());

    std::filesystem::path installPath(taskDomain);

    for (int i = 0; i < path.numParts(); i++) {
        installPath /= path.partView(i);
    }

    if (dotSuffix != nullptr) {
        installPath.replace_extension(dotSuffix);
    }

    return installPath;
}
