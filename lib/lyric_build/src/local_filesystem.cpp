
#include <lyric_build/local_filesystem.h>
#include <tempo_security/sha256_hash.h>
#include <tempo_utils/file_reader.h>

#include "lyric_build/build_result.h"

lyric_build::LocalFilesystem::LocalFilesystem()
    : m_baseDirectory()
{
}

lyric_build::LocalFilesystem::LocalFilesystem(const std::filesystem::path &baseDirectory)
    : m_baseDirectory(baseDirectory)
{
    TU_ASSERT (!m_baseDirectory.empty());
}

bool
lyric_build::LocalFilesystem::supportsScheme(std::string_view scheme)
{
    if (scheme == "file")
        return true;
    if (scheme.empty() && !m_baseDirectory.empty())
        return true;
    return false;
}

bool
lyric_build::LocalFilesystem::supportsScheme(const tempo_utils::Url &url)
{
    return supportsScheme(url.schemeView());
}

Option<bool>
lyric_build::LocalFilesystem::containsResource(const tempo_utils::Url &url)
{
    if (!url.isValid())
        return {};
    if (!supportsScheme(url))
        return {};
    auto path = url.toFilesystemPath(m_baseDirectory);
    return Option<bool>(exists(path));
}

tempo_utils::Result<Option<lyric_build::Resource>>
lyric_build::LocalFilesystem::fetchResource(const tempo_utils::Url &url)
{
    if (!url.isValid())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid local filesystem resource url '{}'", url.toString());

    if (!supportsScheme(url))
        return Option<Resource>();

    auto path = url.toFilesystemPath(m_baseDirectory);
    if (!exists(path))
        return Option<lyric_build::Resource>();

    tempo_utils::FileReader resourceReader(path);
    if (!resourceReader.isValid())
        return resourceReader.getStatus();
    auto bytes = resourceReader.getBytes();

    Resource resource;
    resource.id = path.string();
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
    const tempo_utils::Url &resourceRoot,
    ResourceMatcherFunc matcherFunc,
    const std::string &token)
{
    if (!resourceRoot.isValid())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid local filesystem resource root '{}'", resourceRoot.toString());
    if (!supportsScheme(resourceRoot))
        return tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kUnimplemented,
            "listResources is unimplemented");

    auto path = resourceRoot.toFilesystemPath(m_baseDirectory);
    if (!exists(path))
        return tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kInternalViolation,
            "resource root {} does not exist", path.string());
    std::filesystem::directory_iterator directoryIterator(path);

    bool isRelativeUrl = resourceRoot.isRelative();

    ResourceList resourceList;
    for (auto &dirEntry : directoryIterator) {
        if (!dirEntry.is_regular_file())
            continue;
        auto &filePath = dirEntry.path();
        if (!matcherFunc(filePath))
            continue;
        if (isRelativeUrl) {
            auto relativePath = filePath.lexically_relative(m_baseDirectory);
            resourceList.resources.push_back(tempo_utils::Url::fromRelative(relativePath.string()));
        } else {
            resourceList.resources.push_back(tempo_utils::Url::fromFilesystemPath(filePath));
        }
    }

    return resourceList;
}

tempo_utils::Result<lyric_build::ResourceList>
lyric_build::LocalFilesystem::listResourcesRecursively(
    const tempo_utils::Url &resourceRoot,
    ResourceMatcherFunc matcherFunc,
    const std::string &token)
{
    if (!resourceRoot.isValid())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid local filesystem resource root '{}'", resourceRoot.toString());
    if (!supportsScheme(resourceRoot))
        return tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kUnimplemented,
            "listResourcesRecursively is unimplemented");

    auto path = resourceRoot.toFilesystemPath(m_baseDirectory);
    if (!exists(path))
        return tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kInternalViolation,
            "resource root {} does not exist", path.string());
    std::filesystem::recursive_directory_iterator directoryIterator(path);

    bool isRelativeUrl = resourceRoot.isRelative();

    ResourceList resourceList;
    for (auto &dirEntry : directoryIterator) {
        if (!dirEntry.is_regular_file())
            continue;
        auto &filePath = dirEntry.path();
        if (!matcherFunc(filePath))
            continue;
        if (isRelativeUrl) {
            auto relativePath = filePath.lexically_relative(m_baseDirectory);
            resourceList.resources.push_back(tempo_utils::Url::fromRelative(relativePath.string()));
        } else {
            resourceList.resources.push_back(tempo_utils::Url::fromFilesystemPath(filePath));
        }
    }

    return resourceList;
}
