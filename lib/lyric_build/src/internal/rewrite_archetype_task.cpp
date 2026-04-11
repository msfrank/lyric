//
// #include <filesystem>
//
// #include <lyric_bootstrap/bootstrap_helpers.h>
// #include <lyric_build/base_task.h>
// #include <lyric_build/build_attrs.h>
// #include <lyric_build/build_state.h>
// #include <lyric_build/build_types.h>
// #include <lyric_build/task_settings.h>
// #include <lyric_build/dependency_loader.h>
// #include <lyric_build/internal/build_macros.h>
// #include <lyric_build/internal/rewrite_archetype_task.h>
// #include <lyric_build/task_utils.h>
// #include <lyric_build/metadata_writer.h>
// #include <lyric_build/task_hasher.h>
// #include <lyric_common/common_conversions.h>
// #include <lyric_common/common_types.h>
// #include <lyric_compiler/lyric_compiler.h>
// #include <lyric_runtime/chain_loader.h>
// #include <lyric_rewriter/macro_rewrite_driver.h>
// #include <tempo_config/base_conversions.h>
// #include <tempo_config/container_conversions.h>
// #include <tempo_tracing/tracing_schema.h>
//
// lyric_build::internal::RewriteArchetypeTask::RewriteArchetypeTask(
//     const BuildGeneration &generation,
//     const TaskKey &key,
//     std::shared_ptr<tempo_tracing::TraceSpan> span)
//     : BaseTask(generation, key, std::move(span))
// {
// }
//
// tempo_utils::Status
// lyric_build::internal::RewriteArchetypeTask::configure(const TaskSettings *config)
// {
//     auto taskId = getId();
//
//     auto modulePath = tempo_utils::UrlPath::fromString(taskId.getId());
//     if (!modulePath.isValid())
//         return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
//             "task key id {} is not a valid relative module location", taskId.getId());
//
//     m_moduleLocation = lyric_common::ModuleLocation::fromString(modulePath.toString());
//
//     lyric_common::ModuleLocationParser preludeLocationParser(lyric_bootstrap::preludeLocation());
//
//     // add dependency on parse_archetype
//     m_parseTarget = TaskKey("parse_archetype", taskId.getId());
//
//     return {};
// }
//
// tempo_utils::Result<lyric_build::TaskHash>
// lyric_build::internal::RewriteArchetypeTask::configureTask(
//     const TaskSettings *config,
//     AbstractVirtualFilesystem *virtualFilesystem)
// {
//     auto key = getKey();
//     auto merged = config->merge(TaskSettings({}, {}, {{getId(), getParams()}}));
//     TU_RETURN_IF_NOT_OK (configure(&merged));
//
//     TaskHasher taskHasher(getKey());
//     taskHasher.hashValue(m_moduleLocation.toString());
//     auto hash = taskHasher.finish();
//
//     return hash;
// }
//
// tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
// lyric_build::internal::RewriteArchetypeTask::checkDependencies()
// {
//     return absl::flat_hash_set<TaskKey>({m_parseTarget});
// }
//
// tempo_utils::Status
// lyric_build::internal::RewriteArchetypeTask::rewriteModule(
//     const std::string &taskHash,
//     const absl::flat_hash_map<TaskKey,TaskData> &depStates,
//     BuildState *buildState)
// {
//     if (!depStates.contains(m_parseTarget))
//         return BuildStatus::forCondition(BuildCondition::kTaskFailure,
//             "missing state for dependent task {}", m_parseTarget.toString());
//
//     // get the archetype artifact from the cache
//     auto artifactCache = buildState->getArtifactCache();
//     auto parseHash = depStates.at(m_parseTarget).getHash();
//     TraceId parseTrace(parseHash, m_parseTarget.getDomain(), m_parseTarget.getId());
//     BuildGeneration generation;
//     TU_ASSIGN_OR_RETURN (generation, artifactCache->loadTrace(parseTrace));
//
//     tempo_utils::UrlPath archetypeArtifactPath;
//     TU_ASSIGN_OR_RETURN (archetypeArtifactPath, convert_module_location_to_artifact_path(
//         m_moduleLocation, lyric_common::kIntermezzoFileDotSuffix));
//     ArtifactId archetypeArtifact(generation, parseHash, archetypeArtifactPath);
//
//     std::shared_ptr<const tempo_utils::ImmutableBytes> content;
//     TU_ASSIGN_OR_RETURN (content, artifactCache->loadContentFollowingLinks(archetypeArtifact));
//     lyric_parser::LyricArchetype archetype(content);
//
//     // configure rewriter
//     lyric_rewriter::LyricRewriter rewriter(m_rewriterOptions);
//
//     // define the module origin
//     auto origin = lyric_common::ModuleLocation::fromString(
//         absl::StrCat("dev.zuri.build://", buildState->getGeneration().toString()));
//
//     // configure loader
//     std::shared_ptr<DependencyLoader> dependencyLoader;
//     TU_ASSIGN_OR_RETURN (dependencyLoader, DependencyLoader::create(origin, depStates, artifactCache, tempDirectory()));
//
//     std::vector<std::shared_ptr<lyric_runtime::AbstractLoader>> loaderChain;
//     loaderChain.push_back(dependencyLoader);
//     loaderChain.push_back(buildState->getLoaderChain());
//     auto loader = std::make_shared<lyric_runtime::ChainLoader>(loaderChain);
//
//     std::shared_ptr<lyric_rewriter::MacroRegistry> macroRegistry;
//     TU_ASSIGN_OR_RETURN (macroRegistry, internal::make_build_macros());
//     macroRegistry->sealRegistry();
//
//     //
//     auto macroRewriteDriverBuilder = std::make_shared<lyric_rewriter::MacroRewriteDriverBuilder>(macroRegistry);
//
//     // generate the rewritten archetype by applying all macros
//     logInfo("rewriting archetype {}",  m_moduleLocation.toString());
//     lyric_parser::LyricArchetype rewritten;
//     TU_ASSIGN_OR_RETURN (rewritten, rewriter.rewriteArchetype(archetype, m_moduleLocation.toUrl(),
//         macroRewriteDriverBuilder, traceDiagnostics()));
//
//     // declare the artifact
//     tempo_utils::UrlPath rewrittenArtifactPath;
//     TU_ASSIGN_OR_RETURN (rewrittenArtifactPath, convert_module_location_to_artifact_path(
//         m_moduleLocation, lyric_common::kIntermezzoFileDotSuffix));
//     ArtifactId rewrittenArtifact(buildState->getGeneration(), taskHash, rewrittenArtifactPath);
//     TU_RETURN_IF_NOT_OK (artifactCache->declareArtifact(rewrittenArtifact));
//
//     // store the archetype metadata in the build cache
//     MetadataWriter writer;
//     TU_RETURN_IF_NOT_OK (writer.configure());
//     writer.putAttr(kLyricBuildContentType, std::string(lyric_common::kIntermezzoContentType));
//     LyricMetadata rewrittenMetadata;
//     TU_ASSIGN_OR_RETURN (rewrittenMetadata, writer.toMetadata());
//
//     //
//     TU_RETURN_IF_NOT_OK (artifactCache->storeMetadata(rewrittenArtifact, rewrittenMetadata));
//
//     // store the rewritten archetype content in the build cache
//     auto rewrittenBytes = rewritten.bytesView();
//     TU_RETURN_IF_NOT_OK (artifactCache->storeContent(rewrittenArtifact, rewrittenBytes));
//
//     logInfo("stored rewritten archetype at {}", rewrittenArtifact.toString());
//
//     return {};
// }
//
// Option<tempo_utils::Status>
// lyric_build::internal::RewriteArchetypeTask::runTask(
//     const std::string &taskHash,
//     const absl::flat_hash_map<TaskKey,TaskData> &depStates,
//     BuildState *buildState)
// {
//     auto status = rewriteModule(taskHash, depStates, buildState);
//     return Option(status);
// }
//
// lyric_build::BaseTask *
// lyric_build::internal::new_rewrite_archetype_task(
//     const BuildGeneration &generation,
//     const TaskKey &key,
//     std::shared_ptr<tempo_tracing::TraceSpan> span)
// {
//     return new RewriteArchetypeTask(generation, key, std::move(span));
// }
