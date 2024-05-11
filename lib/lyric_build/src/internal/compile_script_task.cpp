//#include <absl/strings/escaping.h>
//#include <boost/uuid/uuid_io.hpp>
//
//#include <lyric_compiler/lyric_compiler.h>
//#include <lyric_runtime/chain_loader.h>
//#include <tempo_utils/log_message.h>
//
//#include "build_conversions.h"
//#include "compile_script_task.h"
//#include "task_hasher.h"
//
//CompileScriptTask::CompileScriptTask(const boost::uuids::uuid &generation, const TaskKey &key)
//    : BaseTask(generation, key),
//      m_phase(CompileScriptPhase::SYMBOLIZE_SCRIPT)
//{
//}
//
//lyric_build::BuildStatus
//CompileScriptTask::configure(const ConfigStore *config)
//{
//    auto taskKey = getKey();
//
//    auto taskSection = config->getTaskSection(taskKey);
//
//    AssemblyLocationParser preludeLocationParser;
//    tempo_utils::StringParser scriptDataParser;
//    SymbolUrlParser envSymbolParser;
//
//    // set the compiler core assembly location
//    CHECK_BUILD_STATUS(parse_config(m_compilerOptions.preludeLocation, preludeLocationParser,
//        config, taskKey, "preludeLocation"));
//
//    if (!m_compilerOptions.preludeLocation.isValid())
//        return lyric_build::BuildStatus::invalidConfiguration("missing core location");
//
//    // set the utf8 script data
//    CHECK_BUILD_STATUS(parse_config(m_scriptData, scriptDataParser, taskSection, "scriptData"));
//    if (m_scriptData.empty())
//        return lyric_build::BuildStatus::invalidConfiguration("missing script data");
//
//    // store any script environment symbols
//    auto envSymbols = taskSection.mapAt("envSymbols").toSeq();
//    for (auto iterator = envSymbols.seqBegin(); iterator != envSymbols.seqEnd(); iterator++) {
//        lyric_runtime::SymbolUrl envSymbol;
//        CHECK_BUILD_STATUS(parse_config(envSymbol, envSymbolParser, iterator->toValue()));
//        m_envSymbols.insert(envSymbol);
//    }
//
//    return lyric_build::BuildStatus::ok();
//}
//
//lyric_build::BuildResult<std::string>
//CompileScriptTask::configureTask(const ConfigStore *config)
//{
//    auto status = configure(config);
//    if (!status.isOk())
//        return lyric_build::BuildResult<std::string>(status);
//
//    TaskHasher configHasher(getKey());
//    configHasher.hashValue(m_compilerOptions.preludeLocation.toString());
//    configHasher.hashValue(m_scriptData);
//
//    // convert the symbol set into a sorted list, then add each symbol to the hash
//    std::vector<lyric_runtime::SymbolUrl> sortedEnvSymbols(m_envSymbols.cbegin(), m_envSymbols.cend());
//    std::sort(sortedEnvSymbols.begin(), sortedEnvSymbols.end());
//    for (const auto &symbolUrl : sortedEnvSymbols) {
//        configHasher.hashValue(symbolUrl.toString());
//    }
//
//    return lyric_build::BuildResult<std::string>(configHasher.finish());
//}
//
//lyric_build::BuildResult<absl::flat_hash_set<TaskKey>>
//CompileScriptTask::checkDependencies()
//{
//    return lyric_build::BuildResult<absl::flat_hash_set<TaskKey>>(m_compileTargets);
//}
//
//lyric_build::BuildStatus
//CompileScriptTask::symbolizeScript(const absl::flat_hash_map<TaskKey,TaskState> &deps, BuildState *buildState)
//{
//    // the script path is based on the script content hash
//    auto scriptHash = Sha256Hash::hash(m_scriptData);
//    std::filesystem::path scriptPath(absl::BytesToHexString(scriptHash));
//    scriptPath.replace_extension(ASSEMBLY_FILE_SUFFIX);
//
//    auto generation = boost::uuids::to_string(buildState->getGeneration().getUuid());
//    m_scriptLocation = AssemblyLocation("dev.zuri.env", generation, scriptPath);
//
//    TU_LOG_INFO << "parsing script" << m_scriptLocation;
//    lyric_parser::LyricParser parser(m_parserOptions);
//    auto parseResult = parser.parseModule(m_scriptData);
//    if (parseResult.isStatus())
//        return lyric_build::BuildStatus::taskFailure(parseResult.getStatus().getMessage());
//    m_archetype = parseResult.getResult();
//
//    TU_LOG_INFO << "symbolizing script" << m_scriptLocation;
//    LyricCompiler compiler(m_compilerOptions);
//    auto *packageLoader = buildState->getPackageLoader();
//    auto *cache = buildState->getCache();
//    ArtifactLoader loader(buildState->getGeneration(), getKey(), cache, packageLoader);
//
//    auto scanResult = compiler.symbolizeModule(&loader, m_scriptLocation, m_archetype);
//    if (scanResult.isStatus())
//        return lyric_build::BuildStatus::taskFailure(scanResult.getStatus().getMessage());
//    auto assembly = scanResult.getResult();
//
//    absl::flat_hash_set<TaskKey> analyzeTargets;
//    for (uint32_t i = 0; i < assembly.numImports(); i++) {
//        auto location = assembly.getImportLocation(i);
//        if (!location.isValid())
//            return lyric_build::BuildStatus::taskFailure("invalid module link");
//        if (!location.getScheme().empty() || !location.getAuthority().empty())
//            continue;
//        auto symbolizeId = absl::StrCat(location.getPath(), ".", SOURCE_FILE_SUFFIX);
//        analyzeTargets.insert(TaskKey("symbolize_module", symbolizeId));
//    }
//
//    m_compileTargets = analyzeTargets;
//    if (!m_compileTargets.empty())
//        return lyric_build::BuildStatus::taskIncomplete("module has linked symbols");
//    return lyric_build::BuildStatus::ok();
//}
//
//lyric_build::BuildStatus
//CompileScriptTask::compileScript(const absl::flat_hash_map<TaskKey,TaskState> &deps, BuildState *buildState)
//{
////    LOG_DEBUG << "compiling script" << m_scriptLocation;
////    LyricCompiler compiler(m_compilerOptions);
////
////    QVector<AbstractLoader *> loaderChain;
////    auto *scriptEnvironment = buildState->getScriptEnvironment();
////    if (scriptEnvironment)
////        loaderChain.append(scriptEnvironment);
////    loaderChain.append(buildState->getPackageLoader());
////    ChainLoader fallback(loaderChain);
////    auto *cache = buildState->getCache();
////    ArtifactLoader loader(buildState->getGeneration(), getKey(), cache, &fallback);
////
////    auto compileResult = compiler.compileScript(&loader, m_scriptLocation, m_archetype, m_envSymbols);
////    if (compileResult.isStatus())
////        return BuildStatus::taskFailure(compileResult.getStatus().getMessage());
////    auto assembly = compileResult.getResult();
////
////    ArtifactId scriptArtifact(buildState->getGeneration().getId(),
////        getKey(), m_scriptLocation.getPath());
////    auto scriptBytes = assembly.getBytes();
////
////    ArtifactMeta scriptMeta = {
////        {"task-target", AttributeValue(true)},
////        {"create-time", AttributeValue(QDateTime::currentMSecsSinceEpoch())},
////        {"content-type", AttributeValue("application/x.zuri.assembly")},
////    };
////    cache->storeArtifact(scriptArtifact, scriptBytes, scriptMeta);
////    LOG_DEBUG << "stored script" << m_scriptLocation;
////
////    return BuildStatus::ok();
//    return lyric_build::BuildStatus::unimplemented("compile_script");
//}
//
//lyric_build::BuildStatus
//CompileScriptTask::runTask(const absl::flat_hash_map<TaskKey,TaskState> &deps, BuildState *buildState)
//{
//    lyric_build::BuildStatus status;
//    switch (m_phase) {
//        case CompileScriptPhase::SYMBOLIZE_SCRIPT:
//            status = symbolizeScript(deps, buildState);
//            m_phase = CompileScriptPhase::COMPILE_SCRIPT;
//            if (!status.isOk())
//                return status;
//        case CompileScriptPhase::COMPILE_SCRIPT:
//            status =  compileScript(deps, buildState);
//            m_phase = CompileScriptPhase::COMPLETE;
//            return status;
//        case CompileScriptPhase::COMPLETE:
//            return lyric_build::BuildStatus::internalViolation("invalid task phase");
//    }
//}
//
//BaseTask *
//new_compile_script_task(const boost::uuids::uuid &generation, const TaskKey &key)
//{
//    return new CompileScriptTask(generation, key);
//}
