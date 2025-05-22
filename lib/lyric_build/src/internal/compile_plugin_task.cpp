
#include <absl/strings/str_join.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_build/internal/compile_plugin_task.h>
#include <lyric_build/internal/task_utils.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_types.h>
#include <lyric_common/common_conversions.h>
#include <lyric_common/plugin.h>
#include <lyric_compiler/lyric_compiler.h>
#include <lyric_packaging/package_attrs.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/assembler_schema.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/config_serde.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_tracing/tracing_schema.h>
#include <tempo_utils/date_time.h>
#include <tempo_utils/file_reader.h>
#include <tempo_utils/log_message.h>
#include <tempo_utils/process_builder.h>
#include <tempo_utils/process_runner.h>

lyric_build::internal::CompilePluginTask::CompilePluginTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span)
{
}

tempo_utils::Status
lyric_build::internal::CompilePluginTask::configure(const TaskSettings *config)
{
    auto taskId = getId();

    auto modulePath = tempo_utils::UrlPath::fromString(taskId.getId());
    if (!modulePath.isValid())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task key id {} is not a valid relative module location", taskId.getId());

    m_moduleLocation = lyric_common::ModuleLocation::fromString(modulePath.toString());

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

    // parse library names
    tempo_config::StringParser libraryNamesParser;
    tempo_config::SeqTParser libraryNamesListParser(&libraryNamesParser, {});
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_libraryNames, libraryNamesListParser,
        taskSection, "libraryNames"));

    tempo_config::PathParser pathParser;

    // parse include directories
    tempo_config::SeqTParser includeDirectoriesListParser(&pathParser, {});
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_includeDirectories, includeDirectoriesListParser,
        taskSection, "includeDirectories"));

    // parse library directories
    tempo_config::SeqTParser libraryDirectoriesListParser(&pathParser, {});
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_libraryDirectories, libraryDirectoriesListParser,
        taskSection, "libraryDirectories"));

    return {};
}

tempo_utils::Result<std::string>
lyric_build::internal::CompilePluginTask::configureTask(
    const TaskSettings *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto key = getKey();
    auto merged = config->merge(TaskSettings({}, {}, {{getId(), getParams()}}));

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

    std::vector sortedLibraryNames(m_libraryNames.cbegin(), m_libraryNames.cend());
    std::sort(sortedLibraryNames.begin(), sortedLibraryNames.end());
    for (const auto &libraryName: sortedLibraryNames) {
        taskHasher.hashValue(libraryName);
    }

    std::vector sortedIncludeDirectories(m_includeDirectories.cbegin(), m_includeDirectories.cend());
    std::sort(sortedIncludeDirectories.begin(), sortedIncludeDirectories.end());
    for (const auto &includeDirectory: sortedIncludeDirectories) {
        taskHasher.hashValue(includeDirectory.string());
    }

    std::vector sortedLibraryDirectories(m_libraryDirectories.cbegin(), m_libraryDirectories.cend());
    std::sort(sortedLibraryDirectories.begin(), sortedLibraryDirectories.end());
    for (const auto &libraryDirectory: sortedLibraryDirectories) {
        taskHasher.hashValue(libraryDirectory.string());
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
    auto vfs = buildState->getVirtualFilesystem();
    auto *tmp = tempDirectory();

    // copy sources to temp directory
    std::vector<std::filesystem::path> pluginSources;
    for (const auto &pluginSourcePath : m_pluginSourcePaths) {
        Option<Resource> resourceOption;
        TU_ASSIGN_OR_RETURN (resourceOption, vfs->fetchResource(pluginSourcePath));
        if (resourceOption.isEmpty())
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "missing resource {}", pluginSourcePath.toString());
        auto resource = resourceOption.getValue();
        std::shared_ptr<const tempo_utils::ImmutableBytes> content;
        TU_ASSIGN_OR_RETURN (content, vfs->loadResource(resource.id));
        std::filesystem::path pluginSource;
        TU_ASSIGN_OR_RETURN (pluginSource, tmp->putContent(pluginSourcePath, content));
        pluginSources.push_back(std::move(pluginSource));
    }

    // create the parent directories for the plugin in the temp directory
    auto modulePath = m_moduleLocation.getPath();
    std::filesystem::path pluginDirectory;
    TU_ASSIGN_OR_RETURN (pluginDirectory, tmp->makeDirectory(modulePath.getInit()));

    // generate the plugin filename
    auto pluginFilename = lyric_common::pluginFilename(modulePath.getLast().partView());

    // construct the compiler command line
    tempo_utils::ProcessBuilder processBuilder("/usr/bin/cc");
    processBuilder.appendArg("-shared");
    processBuilder.appendArg("-Wall");
    processBuilder.appendArg("-fPIC");
    processBuilder.appendArg("-o", pluginFilename);
    for (const auto &includeDirectory : m_includeDirectories) {
        processBuilder.appendArg(absl::StrCat("-I", includeDirectory.string()));
    }
    for (const auto &libraryDirectory : m_libraryDirectories) {
        processBuilder.appendArg(absl::StrCat("-L", libraryDirectory.string()));
    }
    for (const auto &libraryName : m_libraryNames) {
        processBuilder.appendArg(absl::StrCat("-l", libraryName));
    }
    for (const auto &sourcePath : pluginSources) {
        processBuilder.appendArg(sourcePath.string());
    }

    auto processInvoker = processBuilder.toInvoker();
    std::vector<std::string> processArgs;
    for (int i = 0; i < processInvoker.getArgc(); i++) {
        processArgs.emplace_back(processInvoker.getArg(i));
    }
    auto processCommandline = absl::StrJoin(processArgs, " ");
    TU_LOG_V << "compiler command line: " << processCommandline;

    // compile the plugin
    tempo_utils::ProcessRunner compilerProcess(processInvoker, pluginDirectory);
    TU_RETURN_IF_NOT_OK (compilerProcess.getStatus());
    TU_LOG_V << "compiler output:";
    TU_LOG_V << "----------------";
    TU_LOG_V << compilerProcess.getChildOutput();
    TU_LOG_V << "----------------";
    TU_LOG_V << "compiler error:";
    TU_LOG_V << "----------------";
    TU_LOG_V << compilerProcess.getChildError();
    TU_LOG_V << "----------------";

    auto cache = buildState->getCache();

    // store the plugin content in the cache
    ArtifactId pluginArtifact(buildState->getGeneration().getUuid(), taskHash, m_moduleLocation.toUrl());
    tempo_utils::FileReader reader(pluginDirectory / pluginFilename);
    TU_RETURN_IF_NOT_OK (reader.getStatus());
    TU_RETURN_IF_NOT_OK (cache->storeContent(pluginArtifact, reader.getBytes()));

    // store the outline object metadata in the cache
    MetadataWriter writer;
    writer.putAttr(kLyricBuildEntryType, EntryType::File);
    writer.putAttr(lyric_packaging::kLyricPackagingContentType, std::string("application/octet-stream"));
    writer.putAttr(lyric_packaging::kLyricPackagingCreateTime, tempo_utils::millis_since_epoch());
    writer.putAttr(kLyricBuildModuleLocation, m_moduleLocation);
    auto toMetadataResult = writer.toMetadata();
    if (toMetadataResult.isStatus()) {
        span->logStatus(toMetadataResult.getStatus(), tempo_tracing::LogSeverity::kError);
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "failed to store metadata for {}", pluginArtifact.toString());
    }
    TU_RETURN_IF_NOT_OK (cache->storeMetadata(pluginArtifact, toMetadataResult.getResult()));

    TU_LOG_INFO << "stored plugin at " << pluginArtifact;

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
