
#include <lyric_build/build_result.h>
#include <lyric_build/internal/task_utils.h>
#include <lyric_common/assembly_location.h>
#include <lyric_common/common_types.h>

tempo_utils::Result<lyric_common::AssemblyLocation>
lyric_build::internal::convert_source_url_to_assembly_location(
    const tempo_utils::Url &sourceUrl,
    const tempo_utils::Url &baseUrl)
{
    tempo_utils::Url locationUrl = sourceUrl;

    // if base url is specified then apply it to the source url, resulting in a relative location url
    if (baseUrl.isValid()) {
        auto baseOrigin = baseUrl.toOrigin();
        auto basePath = baseUrl.toPath();
        auto sourcePath = sourceUrl.toPath();

        if (sourceUrl.toOrigin() != baseOrigin || !sourcePath.isDescendentOf(basePath))
            return BuildStatus::forCondition(BuildCondition::kTaskFailure,
                "source url {} is not within the base {}",
                sourceUrl.toString(), baseUrl.toString());

        tempo_utils::UrlPath relativePath;
        for (int i = basePath.numParts(); i < sourcePath.numParts(); i++) {
            relativePath = relativePath.traverse(tempo_utils::UrlPathPart(sourcePath.partView(i)));
        }
        locationUrl = tempo_utils::Url::fromRelative(relativePath.pathView());
    }

    // if location url is relative then remove the source file suffix and return
    if (locationUrl.isRelative()) {
        std::filesystem::path p(locationUrl.toPath().toString(), std::filesystem::path::generic_format);
        if (p.extension() != std::string_view(lyric_common::kSourceFileDotSuffix))
            return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
                "{} is not a valid lyric source file", p.string());
        p.replace_extension();
        return lyric_common::AssemblyLocation(p.string());
    }

    // if location url has a file scheme then remove the source file suffix
    if (locationUrl.getKnownScheme() == tempo_utils::KnownUrlScheme::File) {
        std::filesystem::path p(locationUrl.toPath().toString(), std::filesystem::path::generic_format);
        if (p.extension() != std::string_view(lyric_common::kSourceFileDotSuffix))
            return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
                "{} is not a valid lyric source file", p.string());
        p.replace_extension();  // removes the extension
        locationUrl = tempo_utils::Url::fromAbsolute("file", std::string_view{}, p.string());
    }

    return lyric_common::AssemblyLocation::fromUrl(locationUrl);
}

std::filesystem::path
lyric_build::internal::generate_install_path(
    std::string_view taskDomain,
    const tempo_utils::Url &url,
    const char *dotSuffix)
{
    TU_ASSERT (!taskDomain.empty());
    TU_ASSERT (url.isValid());

    std::filesystem::path installPath(taskDomain);

    if (!url.isRelative()) {
        if (url.hasScheme()) {
            installPath /= url.schemeView();
        } else {
            installPath /= "_";
        }
        if (url.hasOrigin()) {
            installPath /= url.toOrigin().toString();
        } else {
            installPath /= "_";
        }
    }

    auto urlPath = url.toPath();
    for (int i = 0; i < urlPath.numParts(); i++) {
        installPath /= urlPath.partView(i);
    }

    if (dotSuffix != nullptr) {
        installPath.replace_extension(dotSuffix);
    }

    return installPath;
}
