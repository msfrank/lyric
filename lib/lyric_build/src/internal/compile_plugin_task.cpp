
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/config_store.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_build/internal/compile_plugin_task.h>
#include <lyric_build/internal/task_utils.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_types.h>
#include <lyric_common/common_conversions.h>
#include <lyric_compiler/lyric_compiler.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/config_serde.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_tracing/tracing_schema.h>
#include <tempo_utils/log_message.h>
#include <tempo_utils/process_builder.h>
#include <tempo_utils/process_runner.h>

#include "lyric_parser/ast_attrs.h"
#include "lyric_schema/assembler_schema.h"

lyric_build::internal::CompilePluginTask::CompilePluginTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span)
{
}

tempo_utils::Status
lyric_build::internal::CompilePluginTask::configure(const ConfigStore *config)
{
    auto taskId = getId();

    // determine the base url containing plugin source files
    tempo_config::UrlParser pluginSourceBaseUrlParser(tempo_utils::Url{});
    TU_RETURN_IF_NOT_OK(parse_config(m_pluginSourceBaseUrl, pluginSourceBaseUrlParser,
        config, taskId, "pluginSourceBaseUrl"));

    //
    // config below comes only from the task section, it is not resolved from domain or global sections
    //

    auto taskSection = config->getTaskSection(taskId);

    // parse plugin sources
    tempo_config::UrlParser pluginSourceUrlParser;
    tempo_config::SeqTParser<tempo_utils::Url> pluginSourceUrlListParser(&pluginSourceUrlParser, {});
    std::vector<tempo_utils::Url> pluginSourceUrls;
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(pluginSourceUrls, pluginSourceUrlListParser,
        taskSection, "pluginSources"));
    if (pluginSourceUrls.empty())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "no plugin sources specified");

    // if base url is specified then resolve each plugin source url
    if (m_pluginSourceBaseUrl.isValid()) {
        for (const auto &sourceUrl : pluginSourceUrls) {
            m_pluginSourceUrls.push_back(m_pluginSourceBaseUrl.resolve(sourceUrl));
        }
    } else {
        m_pluginSourceUrls = std::move(pluginSourceUrls);
    }

    // parse plugin library names
    tempo_config::StringParser pluginLibraryNameParser;
    tempo_config::SeqTParser<std::string> pluginLibraryNamesListParser(&pluginLibraryNameParser, {});
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_pluginLibraryNames, pluginLibraryNamesListParser,
        taskSection, "pluginLibraries"));

    return {};
}

tempo_utils::Result<std::string>
lyric_build::internal::CompilePluginTask::configureTask(
    const ConfigStore *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto key = getKey();
    auto merged = config->merge({}, {}, {{getId(), getParams()}});

    TU_RETURN_IF_NOT_OK (configure(&merged));

    TaskHasher taskHasher(getKey());

    std::vector sortedPluginSourceUrls(m_pluginSourceUrls.cbegin(), m_pluginSourceUrls.cend());
    std::sort(sortedPluginSourceUrls.begin(), sortedPluginSourceUrls.end(), [](auto &a, auto &b) -> bool {
        return a.toString() < b.toString();
    });
    for (const auto &pluginSourceUrl : sortedPluginSourceUrls) {
        Option<Resource> resourceOption;
        TU_ASSIGN_OR_RETURN (resourceOption, virtualFilesystem->fetchResource(pluginSourceUrl));
        // fail the task if the resource was not found
        if (resourceOption.isEmpty())
            return BuildStatus::forCondition(BuildCondition::kMissingInput,
                "plugin source file {} not found", pluginSourceUrl.toString());
        auto resource = resourceOption.getValue();
        taskHasher.hashValue(resource.entityTag);
    }

    std::vector sortedPluginLibraryNames(m_pluginLibraryNames.cbegin(), m_pluginLibraryNames.cend());
    std::sort(sortedPluginLibraryNames.begin(), sortedPluginLibraryNames.end());
    for (const auto &pluginLibraryName: sortedPluginLibraryNames) {
        taskHasher.hashValue(pluginLibraryName);
    }

    auto hash = taskHasher.finish();

    return hash;
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
lyric_build::internal::CompilePluginTask::checkDependencies()
{
    return absl::flat_hash_set<TaskKey>{};
}

tempo_utils::Status
lyric_build::internal::CompilePluginTask::compilePlugin(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto span = getSpan();

    // construct the compiler command line
    tempo_utils::ProcessBuilder processBuilder("/usr/bin/cc");
    processBuilder.appendArg("-Wall");
    processBuilder.appendArg("-fPIC");
    processBuilder.appendArg("-o", "plugin.so");
    for (const auto &libraryName : m_pluginLibraryNames) {
        processBuilder.appendArg(absl::StrCat("-l", libraryName));
    }
    for (const auto &sourceUrl : m_pluginSourceUrls) {
        processBuilder.appendArg(sourceUrl.toString());
    }

    // compile the plugin
    tempo_utils::ProcessRunner compilerProcess(processBuilder.toInvoker());
    TU_RETURN_IF_NOT_OK (compilerProcess.getStatus());
    TU_LOG_INFO << "compiler output:";
    TU_LOG_INFO << "----------------";
    TU_LOG_INFO << compilerProcess.getChildOutput();
    TU_LOG_INFO << "----------------";
    TU_LOG_INFO << "compiler error:";
    TU_LOG_INFO << "----------------";
    TU_LOG_INFO << compilerProcess.getChildError();
    TU_LOG_INFO << "----------------";

    // store the object content in the build cache

    // generate the install path

    // serialize the object metadata

    // store the object metadata in the build cache

    return {};
}

Option<tempo_utils::Status>
lyric_build::internal::CompilePluginTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto status =  compilePlugin(taskHash, depStates, buildState);
    return Option(status);
}

lyric_build::BaseTask *
lyric_build::internal::new_compile_plugin_task(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new CompilePluginTask(generation, key, span);
}
