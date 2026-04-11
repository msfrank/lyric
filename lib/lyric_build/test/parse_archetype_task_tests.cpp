#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_build/build_attrs.h>
#include <lyric_build/internal/parse_archetype_task.h>
#include <lyric_build/local_filesystem.h>
#include <lyric_build/lyric_builder.h>
#include <lyric_common/common_types.h>
#include <tempo_config/parse_config.h>
#include <tempo_test/result_matchers.h>

#include "base_build_fixture.h"

class ParseArchetypeTask : public BaseBuildFixture {
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
};

TEST_F(ParseArchetypeTask, TaskSucceeds)
{
    writeNamedFile({}, "mod.ly", "nil");

    lyric_build::TaskKey key(std::string("parse_archetype"), std::string("/mod"));
    auto *task = lyric_build::internal::new_parse_archetype_task(generation, key, buildState, span);
    ASSERT_THAT (task->configureTask(taskSettings), tempo_test::IsOk());

    lyric_build::TaskHash taskHash;
    ASSERT_THAT (task->deduplicateTask(taskHash), tempo_test::IsOk());
    ASSERT_TRUE (taskHash.isValid());
    task->setTaskHash(taskHash);

    auto *tmp = tempDirectory();
    ASSERT_THAT (task->runTask(tmp), tempo_test::IsOk());

    auto artifactCache = buildState->getArtifactCache();
    lyric_build::ArtifactId artifactId(generation, taskHash, tempo_utils::Url::fromString("/mod.lyi"));

    auto loadMetadataResult = artifactCache->loadMetadata(artifactId);
    ASSERT_THAT (loadMetadataResult, tempo_test::IsResult());
    auto metadata = loadMetadataResult.getResult();

    std::string contentType;
    metadata.parseAttr(lyric_build::kLyricBuildContentType, contentType);
    ASSERT_EQ (lyric_common::kIntermezzoContentType, contentType);

    auto loadContentResult = artifactCache->loadContent(artifactId);
    ASSERT_THAT (loadContentResult, tempo_test::IsResult());
    auto content = loadContentResult.getResult();

    lyric_parser::LyricArchetype archetype(content);
    ASSERT_TRUE (archetype.isValid());
}

TEST_F(ParseArchetypeTask, ConfigureTaskFailsWhenSourceFileIsMissing)
{
    lyric_build::TaskKey key(std::string("parse_archetype"), std::string("/mod"));
    auto *task = lyric_build::internal::new_parse_archetype_task(generation, key, buildState, span);
    auto status = task->configureTask(taskSettings);
    ASSERT_THAT (status, tempo_test::ContainsStatus(lyric_build::BuildCondition::kMissingInput));
}
