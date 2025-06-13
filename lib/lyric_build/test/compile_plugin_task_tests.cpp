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

TEST_F(CompilePluginTask, RunSucceeds)
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
    auto *task = lyric_build::internal::new_compile_plugin_task(m_generation, key, m_span);
    auto configureTaskResult = task->configureTask(m_config.get(), m_vfs.get());
    ASSERT_THAT (configureTaskResult, tempo_test::IsResult());
    auto taskHash = configureTaskResult.getResult();

    bool taskComplete;
    auto runTaskStatus = task->run(taskHash, {}, m_state.get(), taskComplete);
    ASSERT_TRUE (taskComplete);
    ASSERT_THAT (runTaskStatus, tempo_test::IsOk());
}

TEST_F(CompilePluginTask, ConfigureTaskFailsWhenNoPluginSourcesSpecified)
{
    lyric_build::TaskKey key(std::string("compile_plugin"), std::string("foo"));
    auto *task = lyric_build::internal::new_compile_plugin_task(m_generation, key, m_span);
    tempo_utils::Result<std::string> configureTaskResult = task->configureTask(m_config.get(), m_vfs.get());
    ASSERT_THAT (configureTaskResult, tempo_test::ContainsStatus(lyric_build::BuildCondition::kInvalidConfiguration));
}
