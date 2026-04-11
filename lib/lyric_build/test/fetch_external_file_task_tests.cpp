#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_build/build_attrs.h>
#include <lyric_build/internal/fetch_external_file_task.h>
#include <lyric_build/local_filesystem.h>
#include <lyric_build/lyric_builder.h>
#include <tempo_config/parse_config.h>
#include <tempo_test/result_matchers.h>
#include <tempo_utils/file_writer.h>

#include "base_build_fixture.h"

class FetchExternalFileTask : public BaseBuildFixture {
protected:
    void configure() override {
        tempo_config::ConfigNode rootNode;
        TU_ASSIGN_OR_RAISE (rootNode, tempo_config::read_config_string(R"({
            "global": {},
            "domain": {},
            "tasks": {}
        })"));
        taskSettings = lyric_build::TaskSettings(rootNode.toMap());
    }

    std::filesystem::path writeExternalFile(
        const std::filesystem::path &path,
        std::string_view content) {
        std::filesystem::path absolutePath = path;
        if (path.is_relative()) {
            absolutePath = testerDirectory / path;
        }
        tempo_utils::FileWriter writer(absolutePath, content, tempo_utils::FileWriterMode::CREATE_OR_OVERWRITE);
        if (!writer.isValid()) {
            auto status = tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kInternalViolation,
                "failed to write file {}", absolutePath.string());
            throw tempo_utils::StatusException(status);
        }
        return absolutePath;
    }
};

TEST_F(FetchExternalFileTask, TaskSucceedsWhenProvidedExternalFile)
{
    auto externalFilePath = writeExternalFile("external.txt", "text data");

    lyric_build::TaskKey key(std::string("fetch_external_file"), std::string("/external.txt"),
        tempo_config::ConfigMap{{
            {"filePath", tempo_config::ConfigValue{externalFilePath.string()}},
        }}
    );
    auto *task = lyric_build::internal::new_fetch_external_file_task(generation, key, buildState, span);
    ASSERT_THAT (task->configureTask(taskSettings), tempo_test::IsOk());

    lyric_build::TaskHash taskHash;
    ASSERT_THAT (task->deduplicateTask(taskHash), tempo_test::IsOk());
    ASSERT_TRUE (taskHash.isValid());
    task->setTaskHash(taskHash);

    auto *tmp = tempDirectory();
    ASSERT_THAT (task->runTask(tmp), tempo_test::IsOk());

    auto artifactCache = buildState->getArtifactCache();
    lyric_build::ArtifactId artifactId(generation, taskHash, tempo_utils::Url::fromString("/external.txt"));

    auto loadMetadataResult = artifactCache->loadMetadata(artifactId);
    ASSERT_THAT (loadMetadataResult, tempo_test::IsResult());
    auto metadata = loadMetadataResult.getResult();

    // if contentType is not specified then it should default to application/octet-stream
    std::string contentType;
    metadata.parseAttr(lyric_build::kLyricBuildContentType, contentType);
    ASSERT_EQ ("application/octet-stream", contentType);

    auto loadContentResult = artifactCache->loadContent(artifactId);
    ASSERT_THAT (loadContentResult, tempo_test::IsResult());
    auto content = loadContentResult.getResult();
    std::string_view contentView((const char *) content->getData(), content->getSize());

    ASSERT_EQ ("text data", contentView);
}

TEST_F(FetchExternalFileTask, TaskSucceedsWhenProvidedExternalFileAndArtifactPath)
{
    auto externalFilePath = writeExternalFile("external.txt", "text data");

    lyric_build::TaskKey key(std::string("fetch_external_file"), std::string("/external.txt"),
        tempo_config::ConfigMap{{
            {"filePath", tempo_config::ConfigValue{externalFilePath.string()}},
            {"artifactPath", tempo_config::ConfigValue{"/artifact.txt"}},
        }}
    );
    auto *task = lyric_build::internal::new_fetch_external_file_task(generation, key, buildState, span);
    ASSERT_THAT (task->configureTask(taskSettings), tempo_test::IsOk());

    lyric_build::TaskHash taskHash;
    ASSERT_THAT (task->deduplicateTask(taskHash), tempo_test::IsOk());
    ASSERT_TRUE (taskHash.isValid());
    task->setTaskHash(taskHash);

    auto *tmp = tempDirectory();
    ASSERT_THAT (task->runTask(tmp), tempo_test::IsOk());

    auto artifactCache = buildState->getArtifactCache();
    lyric_build::ArtifactId artifactId(generation, taskHash, tempo_utils::Url::fromString("/artifact.txt"));

    auto loadMetadataResult = artifactCache->loadMetadata(artifactId);
    ASSERT_THAT (loadMetadataResult, tempo_test::IsResult());
    auto metadata = loadMetadataResult.getResult();

    // if contentType is not specified then it should default to application/octet-stream
    std::string contentType;
    metadata.parseAttr(lyric_build::kLyricBuildContentType, contentType);
    ASSERT_EQ ("application/octet-stream", contentType);

    auto loadContentResult = artifactCache->loadContent(artifactId);
    ASSERT_THAT (loadContentResult, tempo_test::IsResult());
    auto content = loadContentResult.getResult();
    std::string_view contentView((const char *) content->getData(), content->getSize());

    ASSERT_EQ ("text data", contentView);
}

TEST_F(FetchExternalFileTask, TaskSucceedsWhenProvidedExternalFileAndContentType)
{
    auto externalFilePath = writeExternalFile("external.txt", "text data");

    lyric_build::TaskKey key(std::string("fetch_external_file"), std::string("/external.txt"),
        tempo_config::ConfigMap{{
            {"filePath", tempo_config::ConfigValue{externalFilePath.string()}},
            {"contentType", tempo_config::ConfigValue{"foo/bar"}},
        }}
    );
    auto *task = lyric_build::internal::new_fetch_external_file_task(generation, key, buildState, span);
    ASSERT_THAT (task->configureTask(taskSettings), tempo_test::IsOk());

    lyric_build::TaskHash taskHash;
    ASSERT_THAT (task->deduplicateTask(taskHash), tempo_test::IsOk());
    ASSERT_TRUE (taskHash.isValid());
    task->setTaskHash(taskHash);

    auto *tmp = tempDirectory();
    ASSERT_THAT (task->runTask(tmp), tempo_test::IsOk());

    auto artifactCache = buildState->getArtifactCache();
    lyric_build::ArtifactId artifactId(generation, taskHash, tempo_utils::Url::fromString("/external.txt"));

    auto loadMetadataResult = artifactCache->loadMetadata(artifactId);
    ASSERT_THAT (loadMetadataResult, tempo_test::IsResult());
    auto metadata = loadMetadataResult.getResult();

    std::string contentType;
    metadata.parseAttr(lyric_build::kLyricBuildContentType, contentType);
    ASSERT_EQ ("foo/bar", contentType);

    auto loadContentResult = artifactCache->loadContent(artifactId);
    ASSERT_THAT (loadContentResult, tempo_test::IsResult());
    auto content = loadContentResult.getResult();
    std::string_view contentView((const char *) content->getData(), content->getSize());

    ASSERT_EQ ("text data", contentView);
}

// TEST_F(FetchExternalFileTask, ConfigureTaskFailsWhenExternalFileIsNotSpecified)
// {
//     lyric_build::TaskKey key(std::string("fetch_external_file"), std::string("/external.txt"),
//         tempo_config::ConfigMap{{
//             {"filePath", tempo_config::ConfigValue{""}},
//         }}
//     );
//     auto *task = lyric_build::internal::new_fetch_external_file_task(m_generation, key, m_span);
//     tempo_utils::Result<std::string> configureTaskResult = task->configureTask(m_config.get(), m_vfs.get());
//     ASSERT_THAT (configureTaskResult, tempo_test::ContainsStatus(lyric_build::BuildCondition::kInvalidConfiguration));
// }

TEST_F(FetchExternalFileTask, ConfigureTaskFailsWhenExternalFileIsNotAnAbsolutePath)
{
    lyric_build::TaskKey key(std::string("fetch_external_file"), std::string("/external.txt"),
        tempo_config::ConfigMap{{
            {"filePath", tempo_config::ConfigValue{"missing.txt"}},
        }}
    );
    auto *task = lyric_build::internal::new_fetch_external_file_task(generation, key, buildState, span);
    auto status = task->configureTask(taskSettings);
    ASSERT_THAT (status, tempo_test::ContainsStatus(lyric_build::BuildCondition::kInvalidConfiguration));
}

TEST_F(FetchExternalFileTask, ConfigureTaskFailsWhenExternalFileIsMissing)
{
    lyric_build::TaskKey key(std::string("fetch_external_file"), std::string("/external.txt"),
        tempo_config::ConfigMap{{
            {"filePath", tempo_config::ConfigValue{"/file/missing.txt"}},
        }}
    );
    auto *task = lyric_build::internal::new_fetch_external_file_task(generation, key, buildState, span);
    auto status = task->configureTask(taskSettings);
    ASSERT_THAT (status, tempo_test::ContainsStatus(lyric_build::BuildCondition::kMissingInput));
}
