
#include <lyric_build/artifact_loader.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/config_store.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_build/internal/compile_module_task.h>
#include <lyric_build/internal/task_utils.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_types.h>
#include <lyric_common/common_conversions.h>
#include <lyric_compiler/lyric_compiler.h>
#include <lyric_packaging/package_attrs.h>
#include <lyric_runtime/chain_loader.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/config_serde.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_tracing/tracing_schema.h>
#include <tempo_utils/date_time.h>
#include <tempo_utils/log_message.h>

#include "lyric_parser/ast_attrs.h"
#include "lyric_schema/assembler_schema.h"

lyric_build::internal::CompileModuleTask::CompileModuleTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span),
      m_phase(CompileModulePhase::ANALYZE_IMPORTS)
{
}

tempo_utils::Status
lyric_build::internal::CompileModuleTask::configure(const ConfigStore *config)
{
    auto taskId = getId();

    auto modulePath = tempo_utils::UrlPath::fromString(taskId.getId());
    if (!modulePath.isValid())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task key id {} is not a valid relative module location", taskId.getId());

    m_moduleLocation = lyric_common::ModuleLocation::fromString(modulePath.toString());

    lyric_common::ModuleLocationParser preludeLocationParser;
    tempo_config::PathParser envSymbolsPathParser(std::filesystem::path{});
    tempo_config::OptionTParser<std::filesystem::path> optEnvSymbolsPathParser(&envSymbolsPathParser);
    tempo_config::BooleanParser touchExternalSymbolsParser(false);

    // set the compiler prelude location
    TU_RETURN_IF_NOT_OK(parse_config(m_objectStateOptions.preludeLocation, preludeLocationParser,
        config, taskId, "preludeLocation"));

    // check for environment symbol map and load it into compiler options if found
    Option<std::filesystem::path> optEnvSymbolsPath;
    TU_RETURN_IF_NOT_OK(parse_config(optEnvSymbolsPath, optEnvSymbolsPathParser,
        config, taskId, "envSymbolsPath"));
    if (!optEnvSymbolsPath.isEmpty()) {
        lyric_common::ModuleLocationParser keyParser;
        lyric_common::SymbolPathParser symbolPathParser;
        tempo_config::SetTParser<lyric_common::SymbolPath> valueParser(&symbolPathParser);
        tempo_config::MapKVParser<
            lyric_common::ModuleLocation,
            absl::flat_hash_set<lyric_common::SymbolPath>
            >
        envSymbolsParser(&keyParser, &valueParser);
        absl::flat_hash_map<lyric_common::ModuleLocation,std::vector<lyric_common::SymbolPath>> envSymbols;
        TU_RETURN_IF_NOT_OK(tempo_config::parse_config_file(m_compilerOptions.envSymbols,
            envSymbolsParser, optEnvSymbolsPath.getValue()));
    }

    // if true, then instruct compiler to add external symbols to links section
    TU_RETURN_IF_NOT_OK(parse_config(m_compilerOptions.touchExternalSymbols, touchExternalSymbolsParser,
        config, taskId, "touchExternalSymbols"));

    // configure the parse_module dependency
    m_parseTarget = TaskKey("parse_module", taskId.getId());

    // configure the symbolize_module dependency
    m_symbolizeTarget = TaskKey("symbolize_module", taskId.getId(), tempo_config::ConfigMap({
        {"preludeLocation", tempo_config::ConfigValue(m_objectStateOptions.preludeLocation.toString())},
    }));

    // set initial dependencies for task
    m_compileTargets = {
        m_parseTarget,
        m_symbolizeTarget,
    };

    return {};
}

tempo_utils::Result<std::string>
lyric_build::internal::CompileModuleTask::configureTask(
    const ConfigStore *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto key = getKey();
    auto merged = config->merge(ConfigStore({}, {}, {{getId(), getParams()}}));

    TU_RETURN_IF_NOT_OK (configure(&merged));

    TaskHasher taskHasher(getKey());
    taskHasher.hashValue(m_objectStateOptions.preludeLocation.toString());
    taskHasher.hashValue(m_moduleLocation.toString());
    auto hash = taskHasher.finish();

    return hash;
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
lyric_build::internal::CompileModuleTask::checkDependencies()
{
    return m_compileTargets;
}

tempo_utils::Status
lyric_build::internal::CompileModuleTask::analyzeImports(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto cache = buildState->getCache();

    if (!depStates.contains(m_parseTarget))
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "missing state for dependent task {}", m_parseTarget.toString());

    const auto &parseHash = depStates.at(m_parseTarget).getHash();
    TraceId parseTrace(parseHash, m_parseTarget.getDomain(), m_parseTarget.getId());
    auto parseGeneration = cache->loadTrace(parseTrace);
    ArtifactId parseArtifact(parseGeneration, parseHash, m_moduleLocation.toUrl());

    std::shared_ptr<const tempo_utils::ImmutableBytes> parseContent;
    TU_ASSIGN_OR_RETURN (parseContent, cache->loadContentFollowingLinks(parseArtifact));
    lyric_parser::LyricArchetype archetype(parseContent);

    // check for a plugin pragma
    lyric_common::ModuleLocation pluginLocation;
    for (int i = 0; i < archetype.numPragmas(); i++) {
        auto pragma = archetype.getPragma(i);
        if (pragma.isClass(lyric_schema::kLyricAssemblerPluginClass)) {
            TU_RETURN_IF_NOT_OK (pragma.parseAttr(lyric_parser::kLyricAstModuleLocation, pluginLocation));
        }
    }

    if (pluginLocation.isValid()) {
        m_pluginTarget = TaskKey("provide_plugin", pluginLocation.getPath().toString());
        m_compileTargets.insert(m_pluginTarget);
    }

    if (!depStates.contains(m_symbolizeTarget))
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "missing state for dependent task {}", m_symbolizeTarget.toString());

    const auto &symbolizeHash = depStates.at(m_symbolizeTarget).getHash();
    TraceId symbolizeTrace(symbolizeHash, m_symbolizeTarget.getDomain(), m_symbolizeTarget.getId());
    auto symbolizeGeneration = cache->loadTrace(symbolizeTrace);
    ArtifactId symbolizeArtifact(symbolizeGeneration, symbolizeHash, m_moduleLocation.toUrl());

    std::shared_ptr<const tempo_utils::ImmutableBytes> symbolizeContent;
    TU_ASSIGN_OR_RETURN (symbolizeContent, cache->loadContentFollowingLinks(symbolizeArtifact));
    lyric_object::LyricObject object(symbolizeContent);

    auto root = object.getObject();

    // check for any imports from modules in the src directory
    absl::flat_hash_set<TaskKey> analyzeTargets;
    for (int i = 0; i < root.numImports(); i++) {
        auto import_ = root.getImport(i);
        auto location = import_.getImportLocation();
        if (!location.isValid())
            return BuildStatus::forCondition(BuildCondition::kTaskFailure,
                "invalid module link {}", location.toString());
        if (location.hasScheme() || location.hasAuthority())    // ignore imports that aren't in the workspace
            continue;
        std::filesystem::path importSourcePath = location.getPath().toString();
        importSourcePath.replace_extension(lyric_common::kSourceFileSuffix);
        analyzeTargets.insert(
            TaskKey("analyze_module", importSourcePath, tempo_config::ConfigMap({
                {"preludeLocation", tempo_config::ConfigValue(m_objectStateOptions.preludeLocation.toString())},
            })
        ));
    }

    m_compileTargets.insert(analyzeTargets.cbegin(), analyzeTargets.cend());

    return {};
}

tempo_utils::Status
lyric_build::internal::CompileModuleTask::compileModule(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    if (!depStates.contains(m_parseTarget))
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "missing state for dependent task {}", m_parseTarget.toString());

    // get the archetype artifact from the cache
    auto cache = buildState->getCache();
    const auto &parseHash = depStates.at(m_parseTarget).getHash();
    TraceId parseTrace(parseHash, m_parseTarget.getDomain(), m_parseTarget.getId());
    auto generation = cache->loadTrace(parseTrace);
    ArtifactId parseArtifact(generation, parseHash, m_moduleLocation.toUrl());

    std::shared_ptr<const tempo_utils::ImmutableBytes> content;
    TU_ASSIGN_OR_RETURN (content, cache->loadContentFollowingLinks(parseArtifact));
    lyric_parser::LyricArchetype archetype(content);

    // construct the local module cache
    std::shared_ptr<lyric_runtime::AbstractLoader> dependencyLoader;
    TU_ASSIGN_OR_RETURN (dependencyLoader, DependencyLoader::create(depStates, cache));
    auto localModuleCache = lyric_importer::ModuleCache::create(dependencyLoader);

    // configure compiler
    lyric_compiler::LyricCompiler compiler(localModuleCache, buildState->getSharedModuleCache(), m_compilerOptions);

    auto span = getSpan();

    // compile the module
    TU_LOG_V << "compiling module " << m_moduleLocation;
    auto compileResult = compiler.compileModule(m_moduleLocation,
        archetype, m_objectStateOptions, traceDiagnostics());
    if (compileResult.isStatus()) {
        span->logStatus(compileResult.getStatus(), absl::Now(), tempo_tracing::LogSeverity::kError);
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "failed to compile module {}", m_moduleLocation.toString());
    }
    auto object = compileResult.getResult();

    // store the object content in the build cache
    ArtifactId objectArtifact(buildState->getGeneration().getUuid(), taskHash, m_moduleLocation.toUrl());
    auto objectBytes = object.bytesView();
    TU_RETURN_IF_NOT_OK (cache->storeContent(objectArtifact, objectBytes));

    // generate the install path
    std::filesystem::path objectInstallPath = generate_install_path(
        getId().getDomain(), m_moduleLocation.getPath(), lyric_common::kObjectFileDotSuffix);

    // serialize the object metadata
    MetadataWriter writer;
    writer.putAttr(kLyricBuildEntryType, EntryType::File);
    writer.putAttr(lyric_packaging::kLyricPackagingContentType, std::string(lyric_common::kObjectContentType));
    writer.putAttr(lyric_packaging::kLyricPackagingCreateTime, tempo_utils::millis_since_epoch());
    writer.putAttr(kLyricBuildModuleLocation, m_moduleLocation);
    writer.putAttr(kLyricBuildInstallPath, objectInstallPath.string());
    auto toMetadataResult = writer.toMetadata();
    if (toMetadataResult.isStatus()) {
        span->logStatus(toMetadataResult.getStatus(), tempo_tracing::LogSeverity::kError);
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "failed to store metadata for {}", objectArtifact.toString());
    }

    // store the object metadata in the build cache
    TU_RETURN_IF_NOT_OK (cache->storeMetadata(objectArtifact, toMetadataResult.getResult()));

    TU_LOG_V << "stored object at " << objectArtifact;

    // if a plugin was provided then pull the plugin artifacts forward
    if (m_pluginTarget.isValid()) {
        TU_ASSERT (depStates.contains(m_pluginTarget));
        const auto &taskState = depStates.at(m_pluginTarget);

        // if the target state is not completed, then fail the task
        if (taskState.getStatus() != TaskState::Status::COMPLETED)
            return BuildStatus::forCondition(BuildCondition::kTaskFailure,
                "dependent task {} did not complete", m_pluginTarget.toString());

        auto hash = taskState.getHash();
        if (hash.empty())
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "dependent task {} has invalid hash", m_pluginTarget.toString());

        TraceId pluginTrace(hash, m_pluginTarget.getDomain(), m_pluginTarget.getId());
        generation = cache->loadTrace(pluginTrace);

        std::vector<ArtifactId> pluginArtifacts;
        TU_ASSIGN_OR_RETURN (pluginArtifacts, cache->findArtifacts(generation, hash, {}, {}));

        for (const auto &srcId : pluginArtifacts) {
            ArtifactId dstId(getGeneration(), taskHash, srcId.getLocation());
            TU_RETURN_IF_NOT_OK (cache->linkArtifact(dstId, srcId));
        }
    }

    return {};
}

Option<tempo_utils::Status>
lyric_build::internal::CompileModuleTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto numDependencies = m_compileTargets.size();
    tempo_utils::Status status;
    switch (m_phase) {
        case CompileModulePhase::ANALYZE_IMPORTS:
            status = analyzeImports(taskHash, depStates, buildState);
            if (!status.isOk())
                return Option(status);
            m_phase = CompileModulePhase::COMPILE_MODULE;
            if (m_compileTargets.size() > numDependencies)
                return {};
            [[fallthrough]];
        case CompileModulePhase::COMPILE_MODULE:
            status =  compileModule(taskHash, depStates, buildState);
            m_phase = CompileModulePhase::COMPLETE;
            return Option(status);
        case CompileModulePhase::COMPLETE:
            status = BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "invalid task phase");
            return Option(status);
        default:
            TU_UNREACHABLE();
    }
}

lyric_build::BaseTask *
lyric_build::internal::new_compile_module_task(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new CompileModuleTask(generation, key, span);
}
