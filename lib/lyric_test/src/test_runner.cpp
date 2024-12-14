#include <iostream>

#include <lyric_build/build_result.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_common/common_types.h>
#include <lyric_runtime/chain_loader.h>
#include <lyric_test/test_runner.h>
#include <lyric_test/test_result.h>
#include <tempo_utils/directory_maker.h>
#include <tempo_utils/file_writer.h>
#include <tempo_utils/log_message.h>
#include <tempo_utils/tempdir_maker.h>
#include <tempo_utils/tempfile_maker.h>

std::shared_ptr<lyric_test::TestRunner>
lyric_test::TestRunner::create(
    const std::filesystem::path &testRootDirectory,
    bool useInMemoryCache,
    bool isTemporary,
    bool keepBuildOnUnexpectedResult,
    const std::string &preludeLocation,
    std::shared_ptr<lyric_runtime::AbstractLoader> fallbackLoader,
    const absl::flat_hash_map<std::string, std::string> &packageMap,
    const tempo_config::ConfigMap &buildConfig,
    const tempo_config::ConfigMap &buildVendorConfig)
{
    return std::shared_ptr<TestRunner>(new TestRunner(
        testRootDirectory,
        useInMemoryCache,
        isTemporary,
        keepBuildOnUnexpectedResult,
        preludeLocation,
        fallbackLoader,
        packageMap,
        buildConfig,
        buildVendorConfig));
}

lyric_test::TestRunner::TestRunner(
    const std::filesystem::path &testRootDirectory,
    bool useInMemoryCache,
    bool isTemporary,
    bool keepBuildOnUnexpectedResult,
    const std::string &preludeLocation,
    std::shared_ptr<lyric_runtime::AbstractLoader> fallbackLoader,
    const absl::flat_hash_map<std::string, std::string> &packageMap,
    const tempo_config::ConfigMap &buildConfig,
    const tempo_config::ConfigMap &buildVendorConfig)
    : std::enable_shared_from_this<TestRunner>(),
      m_testRootDirectory(testRootDirectory),
      m_useInMemoryCache(useInMemoryCache),
      m_isTemporary(isTemporary),
      m_keepBuildOnUnexpectedResult(keepBuildOnUnexpectedResult),
      m_preludeLocation(preludeLocation),
      m_fallbackLoader(std::move(fallbackLoader)),
      m_packageMap(packageMap),
      m_buildConfig(buildConfig),
      m_buildVendorConfig(buildVendorConfig),
      m_configured(false),
      m_builder(nullptr),
      m_unexpectedResult(false)
{
}

lyric_test::TestRunner::~TestRunner()
{
    delete m_builder;

    // disable directory deletion based on the tester options and whether testing generated exceptions
    if (exists(m_testerDirectory)) {
        bool autoRemove = true;
        if (!m_isTemporary)
            autoRemove = false;
        if (m_keepBuildOnUnexpectedResult && m_unexpectedResult)
            autoRemove = false;
        if (autoRemove) {
            TU_LOG_V << "removing tester directory " << m_testerDirectory;
            TU_LOG_WARN_IF (!std::filesystem::remove_all(m_testerDirectory))
                << "failed to remove tester directory " << m_testerDirectory;
        }
    }
}

bool
lyric_test::TestRunner::isConfigured() const
{
    return m_configured;
}

tempo_utils::Status
lyric_test::TestRunner::configureBaseTester()
{
    if (m_configured)
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "tester was already configured");

    absl::flat_hash_map<std::string,tempo_config::ConfigNode> globalOverrides;
    globalOverrides["sourceBaseUrl"] = tempo_config::ConfigValue("/");

    // create a temporary directory for the test build
    std::filesystem::path testRootDirectory;
    if (!m_testRootDirectory.empty()) {
        testRootDirectory = m_testRootDirectory;
    } else {
        testRootDirectory = std::filesystem::current_path();
    }

    tempo_utils::TempdirMaker tempdirMaker(testRootDirectory, "tester.XXXXXXXX");
    if (!tempdirMaker.isValid())
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "failed to create tester directory");

    m_testerDirectory = tempdirMaker.getTempdir();
    TU_LOG_V << "created tester directory " << m_testerDirectory;

    // default to a single builder thread, 1s lock wait timeout
    lyric_build::BuilderOptions builderOptions;
    builderOptions.workspaceRoot = m_testerDirectory;
    builderOptions.numThreads = 1;
    builderOptions.waitTimeoutInMs = 1000;
    builderOptions.fallbackLoader = m_fallbackLoader;

    // create the install directory and configure assembly to user it
    tempo_utils::DirectoryMaker installdirMaker(m_testerDirectory, "install");
    if (!installdirMaker.isValid())
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "failed to create install directory");
    m_installDirectory = installdirMaker.getAbsolutePath();
    builderOptions.installRoot = m_installDirectory;

    // if preludeLocation option is specified, then add it to global overrides
    if (!m_preludeLocation.empty()) {
        globalOverrides["preludeLocation"] = tempo_config::ConfigValue(m_preludeLocation);
    }

    // if packageMap option is specified, then add it to global overrides
    if (!m_packageMap.empty()) {
        absl::flat_hash_map<std::string,tempo_config::ConfigNode> packageMap;
        for (const auto &pair : m_packageMap) {
            packageMap[pair.first] = tempo_config::ConfigValue(pair.second);
        }
        globalOverrides["packageMap"] = tempo_config::ConfigMap(packageMap);
    }

    // if useInMemoryCache or isTemporary is true, then use in memory build cache
    if (m_useInMemoryCache || m_isTemporary) {
        builderOptions.cacheMode = lyric_build::CacheMode::InMemory;
    } else {
        builderOptions.cacheMode = lyric_build::CacheMode::Persistent;
    }

    lyric_build::ConfigStore baseConfigStore = lyric_build::ConfigStore(m_buildConfig, m_buildVendorConfig);
    m_config = baseConfigStore.merge(tempo_config::ConfigMap(globalOverrides), {}, {});

    auto builder = new lyric_build::LyricBuilder(m_config, builderOptions);
    auto status = builder->configure();
    if (status.notOk())
        return status;

    m_builder = builder;
    m_configured = true;
    return status;
}

tempo_utils::Result<std::filesystem::path>
lyric_test::TestRunner::writeModuleInternal(const std::string &code, const std::filesystem::path &path)
{
    if (!m_configured)
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "tester is unconfigured");

    if (!path.empty()) {
        auto relativePath = path;
        relativePath.replace_extension(lyric_common::kSourceFileSuffix);
        return writeNamedFileInternal("src", relativePath, code);
    } else {
        std::filesystem::path templatePath("XXXXXXXX");
        templatePath.replace_extension(lyric_common::kSourceFileSuffix);
        return writeTempFileInternal("src", templatePath, code);
    }
}

tempo_utils::Result<std::filesystem::path>
lyric_test::TestRunner::writeNamedFileInternal(
    const std::filesystem::path &baseDir,
    const std::filesystem::path &path,
    const std::string &code)
{
    auto relativePath = baseDir / path;
    auto absolutePath = m_testerDirectory / relativePath;
    auto parentPath = absolutePath.parent_path();
    std::error_code ec;
    if (!std::filesystem::create_directories(parentPath, ec)) {
        if (ec)
            return TestStatus::forCondition(TestCondition::kTestInvariant,
                "failed to create directory {}: {}", parentPath.string(), ec.message());
    }

    tempo_utils::FileWriter writer(absolutePath.string(), code, tempo_utils::FileWriterMode::CREATE_OR_OVERWRITE);
    if (!writer.isValid())
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "failed to write file {}", absolutePath.string());

    return relativePath;
}

tempo_utils::Result<std::filesystem::path>
lyric_test::TestRunner::writeTempFileInternal(
    const std::filesystem::path &baseDir,
    const std::filesystem::path &templatePath,
    const std::string &code)
{
    auto absolutePath = m_testerDirectory / baseDir / templatePath;
    auto parentPath = absolutePath.parent_path();
    std::error_code ec;
    if (!std::filesystem::create_directories(parentPath, ec)) {
        if (ec)
            return TestStatus::forCondition(TestCondition::kTestInvariant,
                "failed to create directory {}: {}", parentPath.string(), ec.message());
    }

    tempo_utils::TempfileMaker tempfileMaker(absolutePath.parent_path().string(),
        absolutePath.filename().string(), code);
    if (!tempfileMaker.isValid())
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "failed to create temp file {}", absolutePath.string());

    auto relativePath = std::filesystem::relative(tempfileMaker.getTempfile(), m_testerDirectory);
    return relativePath;
}

tempo_utils::Result<lyric_build::TargetComputationSet>
lyric_test::TestRunner::computeTargetInternal(
    const lyric_build::TaskId &target,
    const tempo_config::ConfigMap &globalOverrides,
    const absl::flat_hash_map<std::string,tempo_config::ConfigMap> &domainOverrides,
    const absl::flat_hash_map<lyric_build::TaskId,tempo_config::ConfigMap> &taskOverrides)
{
    TU_CONSOLE_OUT << "";
    TU_CONSOLE_OUT << "======== BUILD: " << target << " ========";
    TU_CONSOLE_OUT << "";

    auto buildResult = m_builder->computeTargets({target}, globalOverrides, domainOverrides, taskOverrides);
    if (buildResult.isStatus())
        return buildResult.getStatus();
    auto targetComputationSet = buildResult.getResult();

    auto diagnostics = targetComputationSet.getDiagnostics();

    auto targetComputation = targetComputationSet.getTarget(target);
    TU_ASSERT (targetComputation.isValid());
    auto taskState = targetComputation.getState();
    diagnostics->printDiagnostics();

    return buildResult;
}

tempo_utils::Result<lyric_test::BuildModule>
lyric_test::TestRunner::buildModuleInternal(
    const std::string &code,
    const std::filesystem::path &path)
{
    if (!m_configured)
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "tester is unconfigured");

    auto writeSourceResult = writeModuleInternal(code, path);
    if (writeSourceResult.isStatus())
        return writeSourceResult.getStatus();
    auto sourcePath = writeSourceResult.getResult();
    lyric_build::TaskId target("compile_module", sourcePath);

    auto buildResult = computeTargetInternal(target, {}, {}, {});
    if (buildResult.isStatus())
        return buildResult.getStatus();
    auto targetComputationSet = buildResult.getResult();
    auto targetComputation = targetComputationSet.getTarget(target);
    TU_ASSERT (targetComputation.isValid());
    auto targetState = targetComputation.getState();
    if (targetState.getStatus() == lyric_build::TaskState::Status::FAILED)
        return BuildModule(shared_from_this(), targetComputation, targetComputationSet.getDiagnostics());

    // construct module location based on the source path
    std::filesystem::path modulePath = "/";
    modulePath /= sourcePath;
    modulePath.replace_extension();
    lyric_common::ModuleLocation moduleLocation(modulePath.string());

    return BuildModule(shared_from_this(), targetComputation,
        targetComputationSet.getDiagnostics(), moduleLocation);
}

tempo_utils::Result<lyric_test::CompileModule>
lyric_test::TestRunner::compileModuleInternal(
    const std::string &code,
    const std::filesystem::path &path)
{
    if (!m_configured)
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "tester is unconfigured");

    auto writeSourceResult = writeModuleInternal(code, path);
    if (writeSourceResult.isStatus())
        return writeSourceResult.getStatus();
    auto sourcePath = writeSourceResult.getResult();
    lyric_build::TaskId target("compile_module", sourcePath);

    auto buildResult = computeTargetInternal(target, {}, {}, {});
    if (buildResult.isStatus())
        return buildResult.getStatus();
    auto targetComputationSet = buildResult.getResult();
    auto targetComputation = targetComputationSet.getTarget(target);
    TU_ASSERT (targetComputation.isValid());
    auto targetState = targetComputation.getState();
    if (targetState.getStatus() == lyric_build::TaskState::Status::FAILED)
        return CompileModule(shared_from_this(), targetComputation, targetComputationSet.getDiagnostics());

    // change the file extension
    std::filesystem::path modulePath = "/";
    modulePath /= sourcePath;
    modulePath.replace_extension();
    lyric_common::ModuleLocation moduleLocation(modulePath.string());

    std::shared_ptr<lyric_build::DependencyLoader> loader;
    TU_ASSIGN_OR_RETURN (loader, lyric_build::DependencyLoader::create(targetComputation, m_builder->getCache()));
    Option<lyric_object::LyricObject> moduleOption;
    TU_ASSIGN_OR_RETURN (moduleOption, loader->loadModule(moduleLocation));
    if (moduleOption.isEmpty())
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "missing module {}", moduleLocation.toString());

    return CompileModule(shared_from_this(), targetComputation,
        targetComputationSet.getDiagnostics(), moduleOption.getValue());
}

tempo_utils::Result<lyric_test::AnalyzeModule>
lyric_test::TestRunner::analyzeModuleInternal(
    const std::string &code,
    const std::filesystem::path &path)
{
    if (!m_configured)
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "tester is unconfigured");

    auto writeSourceResult = writeModuleInternal(code, path);
    if (writeSourceResult.isStatus())
        return writeSourceResult.getStatus();
    auto sourcePath = writeSourceResult.getResult();
    lyric_build::TaskId target("analyze_module", sourcePath);

    auto buildResult = computeTargetInternal(target, {}, {}, {});
    if (buildResult.isStatus())
        return buildResult.getStatus();
    auto targetComputationSet = buildResult.getResult();
    auto targetComputation = targetComputationSet.getTarget(target);
    TU_ASSERT (targetComputation.isValid());
    auto targetState = targetComputation.getState();
    if (targetState.getStatus() == lyric_build::TaskState::Status::FAILED)
        return AnalyzeModule(shared_from_this(), targetComputation, targetComputationSet.getDiagnostics());

    // change the file extension
    std::filesystem::path modulePath = "/";
    modulePath /= sourcePath;
    modulePath.replace_extension();
    lyric_common::ModuleLocation moduleLocation(modulePath.string());

    std::shared_ptr<lyric_build::DependencyLoader> loader;
    TU_ASSIGN_OR_RETURN (loader, lyric_build::DependencyLoader::create(targetComputation, m_builder->getCache()));
    Option<lyric_object::LyricObject> moduleOption;
    TU_ASSIGN_OR_RETURN (moduleOption, loader->loadModule(moduleLocation));
    if (moduleOption.isEmpty())
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "missing module {}", moduleLocation.toString());

    return AnalyzeModule(shared_from_this(), targetComputation,
        targetComputationSet.getDiagnostics(), moduleOption.getValue());
}

tempo_utils::Result<lyric_test::SymbolizeModule>
lyric_test::TestRunner::symbolizeModuleInternal(
    const std::string &code,
    const std::filesystem::path &path)
{
    if (!m_configured)
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "tester is unconfigured");

    auto writeSourceResult = writeModuleInternal(code, path);
    if (writeSourceResult.isStatus())
        return writeSourceResult.getStatus();
    auto sourcePath = writeSourceResult.getResult();
    lyric_build::TaskId target("symbolize_module", sourcePath);

    auto buildResult = computeTargetInternal(target, {}, {}, {});
    if (buildResult.isStatus())
        return buildResult.getStatus();
    auto targetComputationSet = buildResult.getResult();
    auto targetComputation = targetComputationSet.getTarget(target);
    TU_ASSERT (targetComputation.isValid());
    auto targetState = targetComputation.getState();
    if (targetState.getStatus() == lyric_build::TaskState::Status::FAILED)
        return SymbolizeModule(shared_from_this(), targetComputation, targetComputationSet.getDiagnostics());

    // change the file extension
    std::filesystem::path modulePath = "/";
    modulePath /= sourcePath;
    modulePath.replace_extension();
    lyric_common::ModuleLocation moduleLocation(modulePath.string());

    std::shared_ptr<lyric_build::DependencyLoader> loader;
    TU_ASSIGN_OR_RETURN (loader, lyric_build::DependencyLoader::create(targetComputation, m_builder->getCache()));
    Option<lyric_object::LyricObject> moduleOption;
    TU_ASSIGN_OR_RETURN (moduleOption, loader->loadModule(moduleLocation));
    if (moduleOption.isEmpty())
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "missing module {}", moduleLocation.toString());

    return SymbolizeModule(shared_from_this(), targetComputation,
        targetComputationSet.getDiagnostics(), moduleOption.getValue());
}

tempo_utils::Result<lyric_test::PackageModule>
lyric_test::TestRunner::packageModuleInternal(
    const lyric_packaging::PackageSpecifier &specifier,
    const std::string &code,
    const std::filesystem::path &path)
{
    if (!m_configured)
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "tester is unconfigured");

    auto writeSourceResult = writeModuleInternal(code, path);
    if (writeSourceResult.isStatus())
        return writeSourceResult.getStatus();
    auto sourcePath = writeSourceResult.getResult();
    lyric_build::TaskId target("package");

    std::filesystem::path modulePath = "/";
    modulePath /= sourcePath;
    modulePath.replace_extension();

    absl::flat_hash_map<lyric_build::TaskId,tempo_config::ConfigMap> taskOverrides;
    taskOverrides[target] = tempo_config::ConfigMap({
        {"packageDirectory", tempo_config::ConfigValue(m_installDirectory)},
        {"packageName", tempo_config::ConfigValue(specifier.getPackageName())},
        {"packageDomain", tempo_config::ConfigValue(specifier.getPackageDomain())},
        {"majorVersion", tempo_config::ConfigValue(absl::StrCat(specifier.getMajorVersion()))},
        {"minorVersion", tempo_config::ConfigValue(absl::StrCat(specifier.getMinorVersion()))},
        {"patchVersion", tempo_config::ConfigValue(absl::StrCat(specifier.getPatchVersion()))},
        {"mainLocation", tempo_config::ConfigValue(modulePath)},
    });

    auto buildResult = computeTargetInternal(target, {}, {}, taskOverrides);
    if (buildResult.isStatus())
        return buildResult.getStatus();
    auto targetComputationSet = buildResult.getResult();
    auto targetComputation = targetComputationSet.getTarget(target);
    TU_ASSERT (targetComputation.isValid());
    auto targetState = targetComputation.getState();
    if (targetState.getStatus() == lyric_build::TaskState::Status::FAILED)
        return PackageModule(shared_from_this(), targetComputation, targetComputationSet.getDiagnostics());

    return PackageModule(shared_from_this(), targetComputation,
        targetComputationSet.getDiagnostics(), specifier.toUrl());
}

tempo_utils::Result<lyric_test::PackageModule>
lyric_test::TestRunner::packageTargetsInternal(
    const absl::flat_hash_set<lyric_build::TaskId> &targets,
    const lyric_common::ModuleLocation &mainLocation,
    const lyric_packaging::PackageSpecifier &specifier)
{
    if (!m_configured)
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "tester is unconfigured");

    std::vector<tempo_config::ConfigNode> packageTargetsList;
    for (const auto &target : targets) {
        packageTargetsList.push_back(tempo_config::ConfigValue(target.toString()));
    }

    lyric_build::TaskId target("package");

    absl::flat_hash_map<lyric_build::TaskId,tempo_config::ConfigMap> taskOverrides;
    taskOverrides[target] = tempo_config::ConfigMap({
        {"packageTargets", tempo_config::ConfigSeq(packageTargetsList)},
        {"packageDirectory", tempo_config::ConfigValue(m_installDirectory)},
        {"packageName", tempo_config::ConfigValue(specifier.getPackageName())},
        {"packageDomain", tempo_config::ConfigValue(specifier.getPackageDomain())},
        {"majorVersion", tempo_config::ConfigValue(absl::StrCat(specifier.getMajorVersion()))},
        {"minorVersion", tempo_config::ConfigValue(absl::StrCat(specifier.getMinorVersion()))},
        {"patchVersion", tempo_config::ConfigValue(absl::StrCat(specifier.getPatchVersion()))},
        {"mainLocation", tempo_config::ConfigValue(mainLocation.getPath().toString())},
    });

    auto buildResult = computeTargetInternal(target, {}, {}, taskOverrides);
    if (buildResult.isStatus())
        return buildResult.getStatus();
    auto targetComputationSet = buildResult.getResult();
    auto targetComputation = targetComputationSet.getTarget(target);
    TU_ASSERT (targetComputation.isValid());
    auto targetState = targetComputation.getState();
    if (targetState.getStatus() == lyric_build::TaskState::Status::FAILED)
        return PackageModule(shared_from_this(), targetComputation, targetComputationSet.getDiagnostics());

    return PackageModule(shared_from_this(), targetComputation,
        targetComputationSet.getDiagnostics(), specifier.toUrl());
}

tempo_utils::Result<lyric_test::PackageModule>
lyric_test::TestRunner::packageWorkspaceInternal(
    const lyric_common::ModuleLocation &mainLocation,
    const lyric_packaging::PackageSpecifier &specifier)
{
    if (!m_configured)
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "tester is unconfigured");

    lyric_build::TaskId target("package");

    absl::flat_hash_map<lyric_build::TaskId,tempo_config::ConfigMap> taskOverrides;
    taskOverrides[target] = tempo_config::ConfigMap({
        {"packageDirectory", tempo_config::ConfigValue(m_installDirectory)},
        {"packageName", tempo_config::ConfigValue(specifier.getPackageName())},
        {"packageDomain", tempo_config::ConfigValue(specifier.getPackageDomain())},
        {"majorVersion", tempo_config::ConfigValue(absl::StrCat(specifier.getMajorVersion()))},
        {"minorVersion", tempo_config::ConfigValue(absl::StrCat(specifier.getMinorVersion()))},
        {"patchVersion", tempo_config::ConfigValue(absl::StrCat(specifier.getPatchVersion()))},
        {"mainLocation", tempo_config::ConfigValue(mainLocation.getPath().toString())},
    });

    auto buildResult = computeTargetInternal(target, {}, {}, taskOverrides);
    if (buildResult.isStatus())
        return buildResult.getStatus();
    auto targetComputationSet = buildResult.getResult();
    auto targetComputation = targetComputationSet.getTarget(target);
    TU_ASSERT (targetComputation.isValid());
    auto targetState = targetComputation.getState();
    if (targetState.getStatus() == lyric_build::TaskState::Status::FAILED)
        return PackageModule(shared_from_this(), targetComputation, targetComputationSet.getDiagnostics());

    return PackageModule(shared_from_this(), targetComputation,
        targetComputationSet.getDiagnostics(), specifier.toUrl());
}

std::filesystem::path
lyric_test::TestRunner::getTesterDirectory() const
{
    return m_testerDirectory;
}

std::filesystem::path
lyric_test::TestRunner::getInstallDirectory() const
{
    return m_installDirectory;
}

lyric_build::LyricBuilder *
lyric_test::TestRunner::getBuilder() const
{
    return m_builder;
}

tempo_tracing::TempoSpanset
lyric_test::TestRunner::getDiagnostics(const lyric_build::TargetComputation &computation) const
{
    auto targetHash = computation.getState().getHash();
    auto targetId = computation.getId();
    lyric_build::TraceId traceId(targetHash, targetId.getDomain(), targetId.getId());

    auto cache = m_builder->getCache();
    return cache->loadDiagnostics(traceId);
}

bool
lyric_test::TestRunner::containsUnexpectedResult() const
{
    return m_unexpectedResult;
}
void
lyric_test::TestRunner::setUnexpectedResult(bool unexpectedResult)
{
    m_unexpectedResult = unexpectedResult;
}