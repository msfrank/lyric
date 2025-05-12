
#include <absl/strings/escaping.h>
#include <lyric_build/temp_directory.h>
#include <tempo_utils/file_reader.h>
#include <tempo_utils/file_writer.h>

#include "lyric_build/build_result.h"
#include "lyric_build/internal/task_utils.h"

lyric_build::TempDirectory::TempDirectory(
    const std::filesystem::path &tempRoot,
    const BuildGeneration &buildGen,
    const std::string &taskHash)
{
    TU_ASSERT (!tempRoot.empty());
    TU_ASSERT (buildGen.isValid());
    TU_ASSERT (!taskHash.empty());

    m_tempRoot = tempRoot;
    m_genAndHash = absl::StrCat(
        buildGen.getUuid().toString(),
        "_",
        absl::BytesToHexString(taskHash));
}

std::filesystem::path
lyric_build::TempDirectory::getRoot() const
{
    return m_baseDirectory;
}

tempo_utils::Status
lyric_build::TempDirectory::initialize()
{
    if (!m_baseDirectory.empty())
        return {};

    if (!std::filesystem::is_directory(m_tempRoot))
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid temp root {}", m_tempRoot.string());

    auto baseDirectory = m_tempRoot / m_genAndHash;

    // create the temp directory
    std::error_code ec;
    if (!std::filesystem::create_directory(baseDirectory, ec))
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "failed to create temp directory {}: {}", baseDirectory.string(), ec.message());

    // set permissions so only the owner has any access to the directory contents
    using perms = std::filesystem::perms;
    std::filesystem::permissions(baseDirectory, perms::owner_all, ec);
    if (ec)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "failed to set permissions on directory {}: {}", baseDirectory.string(), ec.message());

    m_baseDirectory = std::move(baseDirectory);

    return {};
}

tempo_utils::Result<std::filesystem::path>
lyric_build::TempDirectory::makeDirectory(const tempo_utils::UrlPath &path)
{
    TU_RETURN_IF_NOT_OK (initialize());
    auto fullPath = internal::to_absolute_path_within_base(m_baseDirectory, path);
    if (fullPath.empty())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "directory path {} is outside of the base directory {}",
            path.toString(), m_baseDirectory.string());

    std::error_code ec;
    std::filesystem::create_directory(fullPath, ec);
    if (ec)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "failed to create directory {}: {}", fullPath.string(), ec.message());
    return fullPath;
}

tempo_utils::Result<std::filesystem::path>
lyric_build::TempDirectory::putContent(
    const tempo_utils::UrlPath &path,
    std::shared_ptr<const tempo_utils::ImmutableBytes> content)
{
    TU_RETURN_IF_NOT_OK (initialize());
    auto fullPath = internal::to_absolute_path_within_base(m_baseDirectory, path);
    if (fullPath.empty())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "content path {} is outside of the base directory {}",
            path.toString(), m_baseDirectory.string());

    // create intermediate directories
    auto parentDir = fullPath.parent_path();
    std::error_code ec;
    std::filesystem::create_directory(parentDir, ec);
    if (ec)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "failed to create parent directory {}: {}", parentDir.string(), ec.message());

    // write file content
    std::span bytes(content->getData(), content->getSize());
    tempo_utils::FileWriter writer(fullPath, bytes, tempo_utils::FileWriterMode::CREATE_OR_OVERWRITE);
    TU_RETURN_IF_NOT_OK (writer.getStatus());
    return fullPath;
}

tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>>
lyric_build::TempDirectory::getContent(tempo_utils::UrlPath &path)
{
    TU_RETURN_IF_NOT_OK (initialize());
    auto fullPath = internal::to_absolute_path_within_base(m_baseDirectory, path);
    if (fullPath.empty())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "content path {} is outside of the base directory {}",
            path.toString(), m_baseDirectory.string());

    tempo_utils::FileReader reader(fullPath);
    TU_RETURN_IF_NOT_OK (reader.getStatus());
    return reader.getBytes();
}

tempo_utils::Status
lyric_build::TempDirectory::cleanup()
{
    if (m_baseDirectory.empty())
        return {};

    std::error_code ec;
    std::filesystem::remove_all(m_baseDirectory, ec);
    if (ec)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "failed to remove temp directory {}: {}",
            m_baseDirectory.string(), ec.message());

    m_baseDirectory.clear();
    return {};
}
