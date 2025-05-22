#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_build/internal/provide_plugin_task.h>
#include <lyric_build/local_filesystem.h>
#include <lyric_build/lyric_builder.h>
#include <tempo_config/parse_config.h>
#include <tempo_test/result_matchers.h>

#include "base_build_fixture.h"
#include "lyric_packaging/package_attrs.h"

class ProvidePluginTask : public BaseBuildFixture {
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
};

TEST_F(ProvidePluginTask, RunSucceedsWhenProvidedExternalPluginFile)
{
    writeNamedFile("external", "plugin.lib", "binary data");

    lyric_build::TaskKey key(std::string("provide_plugin"), std::string("/foo"),
        tempo_config::ConfigMap{{
            {
            "existingPluginPath", tempo_config::ConfigValue{"/external/plugin.lib"},
            }
        }}
    );
    auto *task = lyric_build::internal::new_provide_plugin_task(m_generation, key, m_span);
    auto configureTaskResult = task->configureTask(m_config.get(), m_vfs.get());
    ASSERT_THAT (configureTaskResult, tempo_test::IsResult());
    auto taskHash = configureTaskResult.getResult();

    auto runTaskStatusOption = task->run(taskHash, {}, m_state.get());
    ASSERT_TRUE (runTaskStatusOption.hasValue());
    ASSERT_THAT (runTaskStatusOption.getValue(), tempo_test::IsOk());

    auto cache = m_state->getCache();
    lyric_build::ArtifactId artifactId(
        m_state->getGeneration().getUuid(), taskHash, tempo_utils::Url::fromString("/foo"));

    auto loadMetadataResult = cache->loadMetadata(artifactId);
    ASSERT_THAT (loadMetadataResult, tempo_test::IsResult());
    auto metadata = loadMetadataResult.getResult();

    auto walker = metadata.getMetadata();
    std::string contentType;
    walker.parseAttr(lyric_packaging::kLyricPackagingContentType, contentType);
    ASSERT_EQ ("application/octet-stream", contentType);

    auto loadContentResult = cache->loadContent(artifactId);
    ASSERT_THAT (loadContentResult, tempo_test::IsResult());
    auto content = loadContentResult.getResult();
    std::string_view contentView((const char *) content->getData(), content->getSize());

    ASSERT_EQ ("binary data", contentView);
}

TEST_F(ProvidePluginTask, ConfigureTaskFailsWhenExternalPluginIsMissing)
{
    lyric_build::TaskKey key(std::string("provide_plugin"), std::string("/foo"),
        tempo_config::ConfigMap{{
            {
                "existingPluginPath", tempo_config::ConfigValue{"/external/plugin.lib"},
            }
        }}
    );
    auto *task = lyric_build::internal::new_provide_plugin_task(m_generation, key, m_span);
    tempo_utils::Result<std::string> configureTaskResult = task->configureTask(m_config.get(), m_vfs.get());
    ASSERT_THAT (configureTaskResult, tempo_test::ContainsStatus(lyric_build::BuildCondition::kMissingInput));
}