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
    std::shared_ptr<lyric_build::TaskRegistry> taskRegistry,
    std::shared_ptr<lyric_runtime::AbstractLoader> bootstrapLoader,
    std::shared_ptr<lyric_runtime::AbstractLoader> fallbackLoader,
    const lyric_build::TaskSettings &taskSettings)
{
    return std::shared_ptr<TestRunner>(new TestRunner(
        testRootDirectory,
        useInMemoryCache,
        isTemporary,
        keepBuildOnUnexpectedResult,
        taskRegistry,
        bootstrapLoader,
        fallbackLoader,
        taskSettings));
}

lyric_test::TestRunner::TestRunner(
    const std::filesystem::path &testRootDirectory,
    bool useInMemoryCache,
    bool isTemporary,
    bool keepBuildOnUnexpectedResult,
    std::shared_ptr<lyric_build::TaskRegistry> taskRegistry,
    std::shared_ptr<lyric_runtime::AbstractLoader> bootstrapLoader,
    std::shared_ptr<lyric_runtime::AbstractLoader> fallbackLoader,
    const lyric_build::TaskSettings &taskSettings)
    : std::enable_shared_from_this<TestRunner>(),
      m_testRootDirectory(testRootDirectory),
      m_useInMemoryCache(useInMemoryCache),
      m_isTemporary(isTemporary),
      m_keepBuildOnUnexpectedResult(keepBuildOnUnexpectedResult),
      m_taskRegistry(std::move(taskRegistry)),
      m_bootstrapLoader(std::move(bootstrapLoader)),
      m_fallbackLoader(std::move(fallbackLoader)),
      m_taskSettings(taskSettings),
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
    builderOptions.numThreads = 1;
    builderOptions.waitTimeout = absl::Seconds(1);
    builderOptions.bootstrapLoader = m_bootstrapLoader;
    builderOptions.fallbackLoader = m_fallbackLoader;

    // if taskRegistry option is specified, then pass it as a builder option
    if (m_taskRegistry != nullptr) {
        builderOptions.taskRegistry = m_taskRegistry;
    }

    // if useInMemoryCache or isTemporary is true, then use in memory build cache
    if (m_useInMemoryCache || m_isTemporary) {
        builderOptions.cacheMode = lyric_build::CacheMode::InMemory;
    } else {
        builderOptions.cacheMode = lyric_build::CacheMode::Persistent;
    }

    auto builder = new lyric_build::LyricBuilder(m_testerDirectory, m_taskSettings, builderOptions);
    auto status = builder->configure();
    if (status.notOk())
        return status;

    m_builder = builder;
    m_configured = true;
    return status;
}

tempo_utils::Result<std::filesystem::path>
lyric_test::TestRunner::writeNamedFileInternal(
    const std::string &code,
    const std::filesystem::path &filePath,
    const std::filesystem::path &baseDir)
{
    auto relativePath = baseDir.empty()? filePath : baseDir / filePath;
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
    const std::string &code,
    const std::filesystem::path &templatePath,
    const std::filesystem::path &baseDir)
{
    auto relativePath = baseDir.empty()? templatePath : baseDir / templatePath;
    auto absolutePath = m_testerDirectory / relativePath;
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

    return std::filesystem::relative(tempfileMaker.getTempfile(), m_testerDirectory);
}

tempo_utils::Result<lyric_common::ModuleLocation>
lyric_test::TestRunner::writeModuleInternal(
    const std::string &code,
    const std::filesystem::path &modulePath,
    const std::filesystem::path &baseDir)
{
    if (!m_configured)
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "tester is unconfigured");

    std::filesystem::path sourcePath;
    if (!modulePath.empty()) {
        auto relativePath = modulePath;
        relativePath.replace_extension(lyric_common::kSourceFileSuffix);
        TU_ASSIGN_OR_RETURN (sourcePath, writeNamedFileInternal(code, relativePath, baseDir));
    } else {
        std::filesystem::path templatePath("XXXXXXXX");
        templatePath.replace_extension(lyric_common::kSourceFileSuffix);
        TU_ASSIGN_OR_RETURN (sourcePath, writeTempFileInternal(code, templatePath, baseDir));
    }

    std::filesystem::path locationPath = "/";
    locationPath /= sourcePath;
    locationPath.replace_extension();
    return lyric_common::ModuleLocation::fromString(locationPath.string());
}

tempo_utils::Result<lyric_build::TargetComputationSet>
lyric_test::TestRunner::computeTargetInternal(
    const lyric_build::TaskId &target,
    const lyric_build::TaskSettings &overrides)
{
    TU_CONSOLE_OUT << "";
    TU_CONSOLE_OUT << "======== BUILD: " << target << " ========";
    TU_CONSOLE_OUT << "";

    lyric_build::TargetComputationSet targetComputationSet;
    TU_ASSIGN_OR_RETURN (targetComputationSet, m_builder->computeTargets({target}, overrides));

    auto diagnostics = targetComputationSet.getDiagnostics();

    auto targetComputation = targetComputationSet.getTarget(target);
    TU_ASSERT (targetComputation.isValid());
    auto taskState = targetComputation.getState();
    diagnostics->printDiagnostics();

    return targetComputationSet;
}

tempo_utils::Result<lyric_test::BuildModule>
lyric_test::TestRunner::buildModuleInternal(
    const std::string &code,
    const std::filesystem::path &modulePath,
    const std::filesystem::path &baseDir)
{
    if (!m_configured)
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "tester is unconfigured");

    // write the source
    lyric_common::ModuleLocation moduleLocation;
    TU_ASSIGN_OR_RETURN (moduleLocation, writeModuleInternal(code, modulePath, baseDir));

    lyric_build::TaskId target("compile_module", moduleLocation.toString());

    lyric_build::TargetComputationSet targetComputationSet;
    TU_ASSIGN_OR_RETURN (targetComputationSet, computeTargetInternal(target));

    auto targetComputation = targetComputationSet.getTarget(target);
    TU_ASSERT (targetComputation.isValid());
    auto targetState = targetComputation.getState();
    if (targetState.getStatus() == lyric_build::TaskState::Status::FAILED)
        return BuildModule(shared_from_this(), targetComputation, targetComputationSet.getDiagnostics());

    return BuildModule(shared_from_this(), targetComputation,
        targetComputationSet.getDiagnostics(), moduleLocation);
}

tempo_utils::Result<lyric_test::CompileModule>
lyric_test::TestRunner::compileModuleInternal(
    const std::string &code,
    const std::filesystem::path &modulePath,
    const std::filesystem::path &baseDir)
{
    if (!m_configured)
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "tester is unconfigured");

    // write the source
    lyric_common::ModuleLocation moduleLocation;
    TU_ASSIGN_OR_RETURN (moduleLocation, writeModuleInternal(code, modulePath, baseDir));

    lyric_build::TaskId target("compile_module", moduleLocation.toString());

    lyric_build::TargetComputationSet targetComputationSet;
    TU_ASSIGN_OR_RETURN (targetComputationSet, computeTargetInternal(target));

    auto targetComputation = targetComputationSet.getTarget(target);
    TU_ASSERT (targetComputation.isValid());
    auto targetState = targetComputation.getState();
    if (targetState.getStatus() == lyric_build::TaskState::Status::FAILED)
        return CompileModule(shared_from_this(), targetComputation, targetComputationSet.getDiagnostics());

    auto cache = m_builder->getCache();
    auto tempRoot = m_builder->getTempRoot();

    // define the module origin
    auto origin = lyric_common::ModuleLocation::fromString(
        absl::StrCat("dev.zuri.tester://", tempo_utils::UUID::randomUUID().toString()));

    lyric_build::BuildGeneration targetGen(targetState.getGeneration());
    lyric_build::TempDirectory tempDirectory(tempRoot, targetGen);
    std::shared_ptr<lyric_build::DependencyLoader> loader;
    TU_ASSIGN_OR_RETURN (loader, lyric_build::DependencyLoader::create(
        origin, targetComputation, cache, &tempDirectory));

    Option<lyric_object::LyricObject> objectOption;
    auto objectLocation = origin.resolve(moduleLocation);
    TU_ASSIGN_OR_RETURN (objectOption, loader->loadModule(objectLocation));
    if (objectOption.isEmpty())
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "dependency loader is missing object {}", objectLocation.toString());

    return CompileModule(shared_from_this(), targetComputation,
        targetComputationSet.getDiagnostics(), objectOption.getValue());
}

tempo_utils::Result<lyric_test::AnalyzeModule>
lyric_test::TestRunner::analyzeModuleInternal(
    const std::string &code,
    const std::filesystem::path &modulePath,
    const std::filesystem::path &baseDir)
{
    if (!m_configured)
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "tester is unconfigured");

    // write the source
    lyric_common::ModuleLocation moduleLocation;
    TU_ASSIGN_OR_RETURN (moduleLocation, writeModuleInternal(code, modulePath, baseDir));

    lyric_build::TaskId target("analyze_module", moduleLocation.toString());

    lyric_build::TargetComputationSet targetComputationSet;
    TU_ASSIGN_OR_RETURN (targetComputationSet, computeTargetInternal(target));

    auto targetComputation = targetComputationSet.getTarget(target);
    TU_ASSERT (targetComputation.isValid());
    auto targetState = targetComputation.getState();
    if (targetState.getStatus() == lyric_build::TaskState::Status::FAILED)
        return AnalyzeModule(shared_from_this(), targetComputation, targetComputationSet.getDiagnostics());

    auto cache = m_builder->getCache();
    auto tempRoot = m_builder->getTempRoot();

    // define the module origin
    auto origin = lyric_common::ModuleLocation::fromString(
        absl::StrCat("dev.zuri.tester://", tempo_utils::UUID::randomUUID().toString()));

    lyric_build::BuildGeneration targetGen(targetState.getGeneration());
    lyric_build::TempDirectory tempDirectory(tempRoot, targetGen, targetState.getHash());
    std::shared_ptr<lyric_build::DependencyLoader> loader;
    TU_ASSIGN_OR_RETURN (loader, lyric_build::DependencyLoader::create(
        origin, targetComputation, cache, &tempDirectory));

    Option<lyric_object::LyricObject> objectOption;
    auto objectLocation = origin.resolve(moduleLocation);
    TU_ASSIGN_OR_RETURN (objectOption, loader->loadModule(objectLocation));
    if (objectOption.isEmpty())
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "dependency loader is missing object {}", objectLocation.toString());

    return AnalyzeModule(shared_from_this(), targetComputation,
        targetComputationSet.getDiagnostics(), objectOption.getValue());
}

tempo_utils::Result<lyric_test::SymbolizeModule>
lyric_test::TestRunner::symbolizeModuleInternal(
    const std::string &code,
    const std::filesystem::path &modulePath,
    const std::filesystem::path &baseDir)
{
    if (!m_configured)
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "tester is unconfigured");

    // write the source
    lyric_common::ModuleLocation moduleLocation;
    TU_ASSIGN_OR_RETURN (moduleLocation, writeModuleInternal(code, modulePath, baseDir));

    lyric_build::TaskId target("symbolize_module", moduleLocation.toString());

    lyric_build::TargetComputationSet targetComputationSet;
    TU_ASSIGN_OR_RETURN (targetComputationSet, computeTargetInternal(target));

    auto targetComputation = targetComputationSet.getTarget(target);
    TU_ASSERT (targetComputation.isValid());
    auto targetState = targetComputation.getState();
    if (targetState.getStatus() == lyric_build::TaskState::Status::FAILED)
        return SymbolizeModule(shared_from_this(), targetComputation, targetComputationSet.getDiagnostics());

    auto cache = m_builder->getCache();
    auto tempRoot = m_builder->getTempRoot();

    // define the module origin
    auto origin = lyric_common::ModuleLocation::fromString(
        absl::StrCat("dev.zuri.tester://", tempo_utils::UUID::randomUUID().toString()));

    lyric_build::BuildGeneration targetGen(targetState.getGeneration());
    lyric_build::TempDirectory tempDirectory(tempRoot, targetGen, targetState.getHash());
    std::shared_ptr<lyric_build::DependencyLoader> loader;
    TU_ASSIGN_OR_RETURN (loader, lyric_build::DependencyLoader::create(
    origin, targetComputation, cache, &tempDirectory));

    Option<lyric_object::LyricObject> objectOption;
    auto objectLocation = origin.resolve(moduleLocation);
    TU_ASSIGN_OR_RETURN (objectOption, loader->loadModule(objectLocation));
    if (objectOption.isEmpty())
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "dependency loader is missing object {}", objectLocation.toString());

    return SymbolizeModule(shared_from_this(), targetComputation,
        targetComputationSet.getDiagnostics(), objectOption.getValue());
}

std::filesystem::path
lyric_test::TestRunner::getTesterDirectory() const
{
    return m_testerDirectory;
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