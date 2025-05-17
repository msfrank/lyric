
#include <lyric_build/artifact_loader.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/config_store.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_build/internal/analyze_module_task.h>
#include <lyric_build/internal/task_utils.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_conversions.h>
#include <lyric_common/common_types.h>
#include <lyric_compiler/lyric_compiler.h>
#include <lyric_packaging/package_attrs.h>
#include <lyric_runtime/chain_loader.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_tracing/tracing_schema.h>
#include <tempo_utils/date_time.h>
#include <tempo_utils/log_stream.h>

lyric_build::internal::AnalyzeModuleTask::AnalyzeModuleTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span),
      m_phase(AnalyzeModulePhase::SYMBOLIZE_IMPORTS)
{
}

tempo_utils::Status
lyric_build::internal::AnalyzeModuleTask::configure(const ConfigStore *config)
{
    auto taskId = getId();

    auto modulePath = tempo_utils::UrlPath::fromString(taskId.getId());
    if (!modulePath.isValid())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task key id {} is not a valid relative module location", taskId.getId());

    m_moduleLocation = lyric_common::ModuleLocation::fromString(modulePath.toString());

    lyric_common::ModuleLocationParser preludeLocationParser;

    // determine the analyzer prelude location
    TU_RETURN_IF_NOT_OK(parse_config(m_objectStateOptions.preludeLocation, preludeLocationParser,
        config, taskId, "preludeLocation"));

    // configure the parse_module dependency
    m_parseTarget = TaskKey("parse_module", taskId.getId());

    // configure the symbolize_module dependency
    m_symbolizeTarget = TaskKey("symbolize_module", taskId.getId(), tempo_config::ConfigMap({
        {"preludeLocation", tempo_config::ConfigValue(m_objectStateOptions.preludeLocation.toString())},
    }));

    // set initial dependencies for task
    m_analyzeTargets = {
        m_parseTarget,
        m_symbolizeTarget,
    };

    return {};
}

tempo_utils::Result<std::string>
lyric_build::internal::AnalyzeModuleTask::configureTask(
    const ConfigStore *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto merged = config->merge({}, {}, {{getId(), getParams()}});

    TU_RETURN_IF_NOT_OK (configure(&merged));

    TaskHasher taskHasher(getKey());
    taskHasher.hashValue(m_objectStateOptions.preludeLocation.toString());
    taskHasher.hashValue(m_moduleLocation.toString());
    auto hash = taskHasher.finish();

    return hash;
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
lyric_build::internal::AnalyzeModuleTask::checkDependencies()
{
    return m_analyzeTargets;
}

tempo_utils::Status
lyric_build::internal::AnalyzeModuleTask::symbolizeImports(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    if (!depStates.contains(m_symbolizeTarget))
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "missing state for dependent task {}", m_symbolizeTarget.toString());

    auto cache = buildState->getCache();
    const auto &symbolizeHash = depStates.at(m_symbolizeTarget).getHash();
    TraceId symbolizeTrace(symbolizeHash, m_symbolizeTarget.getDomain(), m_symbolizeTarget.getId());
    auto generation = cache->loadTrace(symbolizeTrace);
    ArtifactId symbolizeArtifact(generation, symbolizeHash, m_moduleLocation.toUrl());

    std::shared_ptr<const tempo_utils::ImmutableBytes> content;
    TU_ASSIGN_OR_RETURN (content, cache->loadContentFollowingLinks(symbolizeArtifact));
    lyric_object::LyricObject module(content);

    // check for any imports from modules in the src directory
    absl::flat_hash_set<TaskKey> symbolizeTargets;
    auto object = module.getObject();
    for (int i = 0; i < object.numImports(); i++) {
        auto import_ = object.getImport(i);
        auto location = import_.getImportLocation();
        if (!location.isValid())
            return BuildStatus::forCondition(BuildCondition::kTaskFailure,
                "invalid module import {}", location.toString());
        if (location.hasScheme() || location.hasAuthority())    // ignore imports that aren't in the workspace
            continue;
        std::filesystem::path importSourcePath = location.getPath().toString();
        importSourcePath.replace_extension(lyric_common::kSourceFileSuffix);
        symbolizeTargets.insert(TaskKey("symbolize_module", importSourcePath, tempo_config::ConfigMap({
                {"preludeLocation", tempo_config::ConfigValue(m_objectStateOptions.preludeLocation.toString())},
                {"moduleLocation", tempo_config::ConfigValue(m_moduleLocation.toString())},
            })));
    }

    m_analyzeTargets.insert(symbolizeTargets.cbegin(), symbolizeTargets.cend());

    return {};
}

tempo_utils::Status
lyric_build::internal::AnalyzeModuleTask::analyzeModule(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    if (!depStates.contains(m_parseTarget))
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "missing state for dependent task {}", m_parseTarget.toString());

    // get the archetype artifact from the cache
    auto cache = buildState->getCache();
    auto parseHash = depStates.at(m_parseTarget).getHash();
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

    // configure analyzer
    lyric_analyzer::LyricAnalyzer analyzer(localModuleCache, buildState->getSharedModuleCache(), m_analyzerOptions);

    auto span = getSpan();

    // generate the outline object by analyzing the archetype
    TU_LOG_INFO << "analyzing module " << m_moduleLocation;
    auto scanResult = analyzer.analyzeModule(m_moduleLocation,
        archetype, m_objectStateOptions, traceDiagnostics());
    if (scanResult.isStatus()) {
        span->logStatus(scanResult.getStatus(), tempo_tracing::LogSeverity::kError);
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "failed to analyze module {}", m_moduleLocation.toString());
    }
    auto object = scanResult.getResult();

    // store the outline object content in the cache
    ArtifactId outlineArtifact(buildState->getGeneration().getUuid(), taskHash, m_moduleLocation.toUrl());
    auto outlineBytes = object.bytesView();
    TU_RETURN_IF_NOT_OK (cache->storeContent(outlineArtifact, outlineBytes));

    // generate the install path
    std::filesystem::path outlineInstallPath = generate_install_path(
        getId().getDomain(), m_moduleLocation.getPath(), lyric_common::kObjectFileDotSuffix);

    // store the outline object metadata in the cache
    MetadataWriter writer;
    writer.putAttr(kLyricBuildEntryType, EntryType::File);
    writer.putAttr(lyric_packaging::kLyricPackagingContentType, std::string(lyric_common::kObjectContentType));
    writer.putAttr(lyric_packaging::kLyricPackagingCreateTime, tempo_utils::millis_since_epoch());
    writer.putAttr(kLyricBuildModuleLocation, m_moduleLocation);
    writer.putAttr(kLyricBuildInstallPath, outlineInstallPath.string());
    auto toMetadataResult = writer.toMetadata();
    if (toMetadataResult.isStatus()) {
        span->logStatus(toMetadataResult.getStatus(), tempo_tracing::LogSeverity::kError);
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "failed to store metadata for {}", outlineArtifact.toString());
    }
    TU_RETURN_IF_NOT_OK (cache->storeMetadata(outlineArtifact, toMetadataResult.getResult()));

    TU_LOG_INFO << "stored outline at " << outlineArtifact;

    return {};
}

Option<tempo_utils::Status>
lyric_build::internal::AnalyzeModuleTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto numDependencies = m_analyzeTargets.size();
    tempo_utils::Status status;
    switch (m_phase) {
        case AnalyzeModulePhase::SYMBOLIZE_IMPORTS:
            status = symbolizeImports(taskHash, depStates, buildState);
            if (!status.isOk())
                return Option(status);
            m_phase = AnalyzeModulePhase::ANALYZE_MODULE;
            if (m_analyzeTargets.size() > numDependencies)
                return {};
            [[fallthrough]];
        case AnalyzeModulePhase::ANALYZE_MODULE:
            status =  analyzeModule(taskHash, depStates, buildState);
            m_phase = AnalyzeModulePhase::COMPLETE;
            return Option(status);
        case AnalyzeModulePhase::COMPLETE:
            status = BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "invalid task phase");
            return Option(status);
        default:
            TU_UNREACHABLE();
    }
}

lyric_build::BaseTask *
lyric_build::internal::new_analyze_module_task(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new AnalyzeModuleTask(generation, key, span);
}