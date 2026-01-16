
#include <lyric_build/local_filesystem.h>
#include <tempo_security/sha256_hash.h>
#include <tempo_utils/file_reader.h>

#include "lyric_build/build_result.h"
#include "lyric_build/internal/task_utils.h"

lyric_build::LocalFilesystem::LocalFilesystem(const std::filesystem::path &baseDirectory, bool allowSymlinksOutsideBase)
    : m_baseDirectory(baseDirectory),
      m_allowSymlinksOutsideBase(allowSymlinksOutsideBase)
{
    TU_ASSERT (m_baseDirectory.is_absolute());
}

tempo_utils::Result<std::shared_ptr<lyric_build::LocalFilesystem>>
lyric_build::LocalFilesystem::create(const std::filesystem::path &baseDirectory, bool allowSymlinksOutsideBase)
{
    if (!baseDirectory.is_absolute())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "{} is not an absolute path", baseDirectory.string());
    if (!std::filesystem::is_directory(baseDirectory))
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "{} is not a directory", baseDirectory.string());
    auto absolutePath = std::filesystem::absolute(baseDirectory.lexically_normal());
    return std::shared_ptr<LocalFilesystem>(new LocalFilesystem(absolutePath, allowSymlinksOutsideBase));
}

bool
lyric_build::LocalFilesystem::containsResource(const tempo_utils::UrlPath &path)
{
    if (!path.isValid())
        return false;
    auto resourcePath = internal::to_absolute_path(m_baseDirectory, path, m_allowSymlinksOutsideBase);
    if (resourcePath.empty())
        return false;
    return std::filesystem::is_regular_file(resourcePath);
}

tempo_utils::Result<Option<lyric_build::Resource>>
lyric_build::LocalFilesystem::fetchResource(const tempo_utils::UrlPath &path)
{
    if (!path.isValid())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid local filesystem resource path '{}'", path.toString());

    auto resourcePath = internal::to_absolute_path(m_baseDirectory, path, m_allowSymlinksOutsideBase);
    if (resourcePath.empty())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "path {} is outside of the base directory {}",
            path.toString(), m_baseDirectory.string());

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

    auto rootPath = internal::to_absolute_path(m_baseDirectory, root, m_allowSymlinksOutsideBase);
    if (rootPath.empty())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "resource root {} is outside of the base directory {}",
            root.toString(), m_baseDirectory.string());

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

    auto rootPath = internal::to_absolute_path(m_baseDirectory, root, m_allowSymlinksOutsideBase);
    if (rootPath.empty())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "resource root {} is outside of the base directory {}",
            root.toString(), m_baseDirectory.string());

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
