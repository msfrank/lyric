#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_build/build_attrs.h>
#include <lyric_build/internal/parse_module_task.h>
#include <lyric_build/local_filesystem.h>
#include <lyric_build/lyric_builder.h>
#include <lyric_common/common_types.h>
#include <tempo_config/parse_config.h>
#include <tempo_test/result_matchers.h>

#include "base_build_fixture.h"

class ParseModuleTask : public BaseBuildFixture {
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

TEST_F(ParseModuleTask, ConfigureTask)
{
    writeNamedFile({}, "mod.ly", "nil");

    lyric_build::TaskKey key(std::string("parse_module"), std::string("/mod"));
    auto *task = lyric_build::internal::new_parse_module_task(m_generation, key, m_span);
    auto configureTaskResult = task->configureTask(m_config.get(), m_vfs.get());
    ASSERT_THAT (configureTaskResult, tempo_test::IsResult());
    auto taskHash = configureTaskResult.getResult();

    auto runTaskStatusOption = task->runTask(taskHash, {}, m_state.get());
    ASSERT_TRUE (runTaskStatusOption.hasValue());
    ASSERT_THAT (runTaskStatusOption.getValue(), tempo_test::IsOk());

    auto cache = m_state->getCache();
    lyric_build::ArtifactId artifactId(
        m_state->getGeneration().getUuid(), taskHash, tempo_utils::Url::fromString("/mod.lyi"));

    auto loadMetadataResult = cache->loadMetadata(artifactId);
    ASSERT_THAT (loadMetadataResult, tempo_test::IsResult());
    auto metadata = loadMetadataResult.getResult();

    auto walker = metadata.getMetadata();
    std::string contentType;
    walker.parseAttr(lyric_build::kLyricBuildContentType, contentType);
    ASSERT_EQ (lyric_common::kIntermezzoContentType, contentType);

    auto loadContentResult = cache->loadContent(artifactId);
    ASSERT_THAT (loadContentResult, tempo_test::IsResult());
    auto content = loadContentResult.getResult();

    lyric_parser::LyricArchetype archetype(content);
    ASSERT_TRUE (archetype.isValid());
}

TEST_F(ParseModuleTask, ConfigureTaskFailsWhenSourceFileIsMissing)
{
    lyric_build::TaskKey key(std::string("parse_module"), std::string("/mod"));
    auto *task = lyric_build::internal::new_parse_module_task(m_generation, key, m_span);
    tempo_utils::Result<std::string> configureTaskResult = task->configureTask(m_config.get(), m_vfs.get());
    ASSERT_THAT (configureTaskResult, tempo_test::ContainsStatus(lyric_build::BuildCondition::kMissingInput));
}
