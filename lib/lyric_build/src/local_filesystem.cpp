
#include <lyric_build/local_filesystem.h>
#include <tempo_security/sha256_hash.h>
#include <tempo_utils/file_reader.h>

#include "lyric_build/build_result.h"

lyric_build::LocalFilesystem::LocalFilesystem(const std::filesystem::path &baseDirectory)
    : m_baseDirectory(baseDirectory)
{
    TU_ASSERT (m_baseDirectory.is_absolute());
}

tempo_utils::Result<std::shared_ptr<lyric_build::LocalFilesystem>>
lyric_build::LocalFilesystem::create(const std::filesystem::path &baseDirectory)
{
    if (!baseDirectory.is_absolute())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "{} is not an absolute path", baseDirectory.string());
    if (!std::filesystem::is_directory(baseDirectory))
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "{} is not a directory", baseDirectory.string());
    auto absolutePath = std::filesystem::absolute(baseDirectory.lexically_normal());
    return std::shared_ptr<LocalFilesystem>(new LocalFilesystem(absolutePath));
}

static tempo_utils::Result<std::filesystem::path>
to_absolute_path(const std::filesystem::path &baseDirectory, const tempo_utils::UrlPath &path)
{
    auto absolutePath = baseDirectory;
    for (int i = 0; i < path.numParts(); i++) {
        absolutePath /= path.getPart(i).partView();
    }
    absolutePath = absolutePath.lexically_normal();
    if (std::filesystem::relative(absolutePath, baseDirectory).string().starts_with(".."))
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kInvalidConfiguration,
            "path {} is outside of the base directory {}", path.toString(), baseDirectory.string());
    return absolutePath;
}

bool
lyric_build::LocalFilesystem::containsResource(const tempo_utils::UrlPath &path)
{
    if (!path.isValid())
        return false;
    auto toAbsoluteResult = to_absolute_path(m_baseDirectory, path);
    if (toAbsoluteResult.isStatus())
        return false;
    auto resourcePath = toAbsoluteResult.getResult();
    return std::filesystem::is_regular_file(resourcePath);
}

tempo_utils::Result<Option<lyric_build::Resource>>
lyric_build::LocalFilesystem::fetchResource(const tempo_utils::UrlPath &path)
{
    if (!path.isValid())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid local filesystem resource path '{}'", path.toString());

    std::filesystem::path resourcePath;
    TU_ASSIGN_OR_RETURN (resourcePath, to_absolute_path(m_baseDirectory, path));
    if (!std::filesystem::is_regular_file(resourcePath))
        return Option<Resource>();

    tempo_utils::FileReader resourceReader(resourcePath);
    TU_RETURN_IF_NOT_OK (resourceReader.getStatus());
    auto bytes = resourceReader.getBytes();

    Resource resource;
    resource.id = resourcePath.string();
    resource.entityTag = tempo_security::Sha256Hash::hash(
        std::string_view((const char *) bytes->getData(), bytes->getSize()));
    resource.lastModifiedMillis = 0;

    return Option(resource);
}

tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>>
lyric_build::LocalFilesystem::loadResource(std::string_view resourceId)
{
    std::filesystem::path path(resourceId);
    tempo_utils::FileReader resourceReader(path);
    if (!resourceReader.isValid())
        return resourceReader.getStatus();
    auto bytes = resourceReader.getBytes();
    return bytes;
}

tempo_utils::Result<lyric_build::ResourceList>
lyric_build::LocalFilesystem::listResources(
    const tempo_utils::UrlPath &root,
    ResourceMatcherFunc matcherFunc,
    const std::string &token)
{
    if (!root.isValid())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid local filesystem resource root '{}'", root.toString());

    std::filesystem::path rootPath;
    TU_ASSIGN_OR_RETURN (rootPath, to_absolute_path(m_baseDirectory, root));
    if (!std::filesystem::is_directory(rootPath))
        return tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kInternalViolation,
            "resource root {} is not a valid directory", rootPath.string());

    std::filesystem::directory_iterator directoryIterator(rootPath);

    ResourceList resourceList;
    for (auto &dirEntry : directoryIterator) {
        if (!dirEntry.is_regular_file())
            continue;
        auto &filePath = dirEntry.path();
        auto relativePath = std::filesystem::relative(filePath, m_baseDirectory);
        auto matchPath = tempo_utils::UrlPath::fromString(absl::StrCat("/", relativePath.string()));
        if (!matcherFunc(matchPath))
            continue;
        resourceList.resources.push_back(matchPath);
    }

    return resourceList;
}

tempo_utils::Result<lyric_build::ResourceList>
lyric_build::LocalFilesystem::listResourcesRecursively(
    const tempo_utils::UrlPath &root,
    ResourceMatcherFunc matcherFunc,
    const std::string &token)
{
    if (!root.isValid())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid local filesystem resource root '{}'", root.toString());

    std::filesystem::path rootPath;
    TU_ASSIGN_OR_RETURN (rootPath, to_absolute_path(m_baseDirectory, root));
    if (!std::filesystem::is_directory(rootPath))
        return tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kInternalViolation,
            "resource root {} is not a valid directory", rootPath.string());

    std::filesystem::recursive_directory_iterator directoryIterator(rootPath);

    ResourceList resourceList;
    for (auto &dirEntry : directoryIterator) {
        if (!dirEntry.is_regular_file())
            continue;
        auto &filePath = dirEntry.path();
        auto relativePath = std::filesystem::relative(filePath, m_baseDirectory);
        auto matchPath = tempo_utils::UrlPath::fromString(absl::StrCat("/", relativePath.string()));
        if (!matcherFunc(matchPath))
            continue;
        resourceList.resources.push_back(matchPath);
    }

    return resourceList;
}
