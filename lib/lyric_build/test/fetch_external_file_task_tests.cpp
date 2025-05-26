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
#include "lyric_common/common_types.h"

class FetchExternalFileTask : public BaseBuildFixture {
protected:
    tempo_utils::Status configure() override {
        tempo_config::ConfigNode rootNode;
        TU_ASSIGN_OR_RAISE (rootNode, tempo_config::read_config_string(R"({
            "global": {},
            "domain": {},
            "tasks": {}
        })"));
        m_config = std::make_unique<lyric_build::TaskSettings>(rootNode.toMap());
        return {};

    }

    std::filesystem::path writeExternalFile(
        const std::filesystem::path &path,
        std::string_view content) {
        std::filesystem::path absolutePath = path;
        if (path.is_relative()) {
            absolutePath = m_testerDirectory / path;
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

TEST_F(FetchExternalFileTask, RunSucceedsWhenProvidedExternalPluginFile)
{
    auto externalFilePath = writeExternalFile("external.txt", "text data");

    lyric_build::TaskKey key(std::string("fetch_external_file"), std::string("/external.txt"),
        tempo_config::ConfigMap{{
            {
            "filePath", tempo_config::ConfigValue{externalFilePath.string()},
            }
        }}
    );
    auto *task = lyric_build::internal::new_fetch_external_file_task(m_generation, key, m_span);
    auto configureTaskResult = task->configureTask(m_config.get(), m_vfs.get());
    ASSERT_THAT (configureTaskResult, tempo_test::IsResult());
    auto taskHash = configureTaskResult.getResult();

    auto runTaskStatusOption = task->run(taskHash, {}, m_state.get());
    ASSERT_TRUE (runTaskStatusOption.hasValue());
    ASSERT_THAT (runTaskStatusOption.getValue(), tempo_test::IsOk());

    auto cache = m_state->getCache();
    lyric_build::ArtifactId artifactId(
        m_state->getGeneration().getUuid(), taskHash, tempo_utils::Url::fromString("/external.txt"));

    auto loadMetadataResult = cache->loadMetadata(artifactId);
    ASSERT_THAT (loadMetadataResult, tempo_test::IsResult());
    auto metadata = loadMetadataResult.getResult();

    auto walker = metadata.getMetadata();
    std::string contentType;
    walker.parseAttr(lyric_build::kLyricBuildContentType, contentType);
    ASSERT_EQ ("application/octet-stream", contentType);

    auto loadContentResult = cache->loadContent(artifactId);
    ASSERT_THAT (loadContentResult, tempo_test::IsResult());
    auto content = loadContentResult.getResult();
    std::string_view contentView((const char *) content->getData(), content->getSize());

    ASSERT_EQ ("text data", contentView);
}

TEST_F(FetchExternalFileTask, ConfigureTaskFailsWhenExternalPluginIsMissing)
{
    lyric_build::TaskKey key(std::string("fetch_external_file"), std::string("/external.txt"),
        tempo_config::ConfigMap{{
            {
                "filePath", tempo_config::ConfigValue{"/file/missing.txt"},
            }
        }}
    );
    auto *task = lyric_build::internal::new_fetch_external_file_task(m_generation, key, m_span);
    tempo_utils::Result<std::string> configureTaskResult = task->configureTask(m_config.get(), m_vfs.get());
    ASSERT_THAT (configureTaskResult, tempo_test::ContainsStatus(lyric_build::BuildCondition::kMissingInput));
}
