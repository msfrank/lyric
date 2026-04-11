#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_build/internal/compile_plugin_task.h>
#include <lyric_build/local_filesystem.h>
#include <lyric_build/lyric_builder.h>
#include <tempo_config/parse_config.h>
#include <tempo_test/result_matchers.h>

#include "base_build_fixture.h"

class CompilePluginTask : public BaseBuildFixture {
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

TEST_F(CompilePluginTask, TaskSucceeds)
{
    writeNamedFile("plugin", "foo_plugin.cpp", R"(
        int forty_two() { return 42; }
    )");

    lyric_build::TaskKey key(std::string("compile_plugin"), std::string("foo"),
        tempo_config::ConfigMap{{
            {
                "pluginSourceBasePath", tempo_config::ConfigValue{"/plugin"}
            },
        {
            "pluginSources", tempo_config::ConfigSeq{{
               tempo_config::ConfigValue{"foo_plugin.cpp"},
            }},
        }
    }});
    auto *task = lyric_build::internal::new_compile_plugin_task(generation, key, buildState, span);
    ASSERT_THAT (task->configureTask(taskSettings), tempo_test::IsOk());

    lyric_build::TaskHash taskHash;
    ASSERT_THAT (task->deduplicateTask(taskHash), tempo_test::IsOk());
    ASSERT_TRUE (taskHash.isValid());
    task->setTaskHash(taskHash);

    auto *tmp = tempDirectory();
    ASSERT_THAT (task->runTask(tmp), tempo_test::IsOk());
}

TEST_F(CompilePluginTask, ConfigureTaskFailsWhenNoPluginSourcesSpecified)
{
    lyric_build::TaskKey key(std::string("compile_plugin"), std::string("foo"));
    auto *task = lyric_build::internal::new_compile_plugin_task(generation, key, buildState, span);
    auto status = task->configureTask(taskSettings);
    ASSERT_THAT (status, tempo_test::ContainsStatus(lyric_build::BuildCondition::kInvalidConfiguration));
}
