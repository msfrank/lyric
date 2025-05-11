
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

    // determine the base path containing plugin source files
    tempo_config::UrlPathParser pluginSourceBasePathParser(tempo_utils::UrlPath{});
    TU_RETURN_IF_NOT_OK(parse_config(m_pluginSourceBasePath, pluginSourceBasePathParser,
        config, taskId, "pluginSourceBasePath"));

    //
    // config below comes only from the task section, it is not resolved from domain or global sections
    //

    auto taskSection = config->getTaskSection(taskId);

    // parse plugin sources
    tempo_config::UrlPathParser pluginSourcePathParser;
    tempo_config::SeqTParser pluginSourcePathListParser(&pluginSourcePathParser, {});
    std::vector<tempo_utils::UrlPath> pluginSourcePaths;
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(pluginSourcePaths, pluginSourcePathListParser,
        taskSection, "pluginSources"));
    if (pluginSourcePaths.empty())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "no plugin sources specified");

    // if base url is specified then resolve each plugin source url
    if (m_pluginSourceBasePath.isValid()) {
        for (const auto &sourcePath : pluginSourcePaths) {
            auto fullPath = build_full_path(sourcePath, m_pluginSourceBasePath);
            m_pluginSourcePaths.push_back(fullPath);
        }
    } else {
        m_pluginSourcePaths = std::move(pluginSourcePaths);
    }

    // parse plugin library names
    tempo_config::StringParser pluginLibraryNameParser;
    tempo_config::SeqTParser pluginLibraryNamesListParser(&pluginLibraryNameParser, {});
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

    std::vector sortedPluginSourcePaths(m_pluginSourcePaths.cbegin(), m_pluginSourcePaths.cend());
    std::sort(sortedPluginSourcePaths.begin(), sortedPluginSourcePaths.end(), [](auto &a, auto &b) -> bool {
        return a.toString() < b.toString();
    });
    for (const auto &pluginSourcePath : sortedPluginSourcePaths) {
        Option<Resource> resourceOption;
        TU_ASSIGN_OR_RETURN (resourceOption, virtualFilesystem->fetchResource(pluginSourcePath));
        // fail the task if the resource was not found
        if (resourceOption.isEmpty())
            return BuildStatus::forCondition(BuildCondition::kMissingInput,
                "plugin source file {} not found", pluginSourcePath.toString());
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
    for (const auto &sourcePath : m_pluginSourcePaths) {
        processBuilder.appendArg(sourcePath.toString());
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
