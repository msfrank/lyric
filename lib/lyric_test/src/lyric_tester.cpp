#include <iostream>
#include <lyric_build/artifact_loader.h>

#include <lyric_build/build_result.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/chain_loader.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/test_inspector.h>
#include <lyric_test/test_result.h>

lyric_test::LyricTester::LyricTester(const TesterOptions &options)
    : m_options(options)
{
    m_runner = TestRunner::create(
        options.testRootDirectory,
        options.useInMemoryCache,
        options.isTemporary,
        options.keepBuildOnUnexpectedResult,
        options.taskRegistry,
        options.fallbackLoader,
        options.taskSettings);
}

tempo_utils::Status
lyric_test::LyricTester::configure()
{
    return m_runner->configureBaseTester();
}

const lyric_test::TestRunner *
lyric_test::LyricTester::getRunner() const
{
    return m_runner.get();
}

tempo_utils::Result<lyric_common::ModuleLocation>
lyric_test::LyricTester::writeModule(
    const std::string &code,
    const std::filesystem::path &modulePath,
    const std::filesystem::path &baseDir)
{
    if (!m_runner->isConfigured())
        return TestStatus::forCondition(TestCondition::kTestInvariant, "tester is unconfigured");
    return m_runner->writeModuleInternal(code, modulePath, baseDir);
}

tempo_utils::Result<lyric_test::SymbolizeModule>
lyric_test::LyricTester::symbolizeModule(
    const std::string &code,
    const std::filesystem::path &modulePath,
    const std::filesystem::path &baseDir)
{
    if (!m_runner->isConfigured())
        return TestStatus::forCondition(TestCondition::kTestInvariant, "tester is unconfigured");
    return m_runner->symbolizeModuleInternal(code, modulePath, baseDir);
}

tempo_utils::Result<lyric_test::AnalyzeModule>
lyric_test::LyricTester::analyzeModule(
    const std::string &code,
    const std::filesystem::path &modulePath,
    const std::filesystem::path &baseDir)
{
    if (!m_runner->isConfigured())
        return TestStatus::forCondition(TestCondition::kTestInvariant, "tester is unconfigured");
    return m_runner->analyzeModuleInternal(code, modulePath, baseDir);
}

tempo_utils::Result<lyric_test::CompileModule>
lyric_test::LyricTester::compileModule(
    const std::string &code,
    const std::filesystem::path &modulePath,
    const std::filesystem::path &baseDir)
{
    if (!m_runner->isConfigured())
        return TestStatus::forCondition(TestCondition::kTestInvariant, "tester is unconfigured");
    return m_runner->compileModuleInternal(code, modulePath, baseDir);
}

tempo_utils::Result<lyric_test::RunModule>
lyric_test::LyricTester::runModule(
    const std::string &code,
    const std::filesystem::path &modulePath,
    const std::filesystem::path &baseDir)
{
   if (!m_runner->isConfigured())
       return TestStatus::forCondition(TestCondition::kTestInvariant,
           "tester is unconfigured");

    // compile the module file
    auto buildResult = m_runner->buildModuleInternal(code, modulePath, baseDir);
    if (buildResult.isStatus())
        return buildResult.getStatus();
    auto testRun = buildResult.getResult();
    if (!testRun.hasLocation())
        return RunModule(m_runner, testRun.getComputation(), testRun.getDiagnostics());

    auto moduleLocation = testRun.getLocation();

    TU_CONSOLE_OUT << "";
    TU_CONSOLE_OUT << "======== RUN: " << moduleLocation << " ========";
    TU_CONSOLE_OUT << "";

    auto *builder = m_runner->getBuilder();
    auto cache = builder->getCache();

    // construct the loader
    auto targetComputation = testRun.getComputation();
    auto targetId = targetComputation.getId();
    auto targetState = targetComputation.getState();
    auto createDependencyLoaderResult = lyric_build::DependencyLoader::create({
        {lyric_build::TaskKey(targetId.getDomain(), targetId.getId()), targetState}},
        cache);
    if (createDependencyLoaderResult.isStatus())
        return createDependencyLoaderResult.getStatus();

    lyric_runtime::InterpreterStateOptions options;

    std::vector<std::shared_ptr<lyric_runtime::AbstractLoader>> loaderChain;
    loaderChain.push_back(builder->getBootstrapLoader());
    loaderChain.push_back(createDependencyLoaderResult.getResult());
    if (m_options.fallbackLoader) {
        loaderChain.push_back(m_options.fallbackLoader);
    }
    options.loader = std::make_shared<lyric_runtime::ChainLoader>(loaderChain);

    // construct the interpreter state
    auto createInterpreterStateResult = lyric_runtime::InterpreterState::create(options, moduleLocation);
    if (createInterpreterStateResult.isStatus())
        return createInterpreterStateResult.getStatus();
    auto state = createInterpreterStateResult.getResult();

    // run the module in the interpreter
    lyric_test::TestInspector inspector;
    lyric_runtime::BytecodeInterpreter interp(state, &inspector);
    auto execResult = interp.run();

    // return the interpreter result
    if (execResult.isStatus())
        return execResult.getStatus();
    return RunModule(m_runner, targetComputation, testRun.getDiagnostics(), state, execResult.getResult());
}

tempo_utils::Result<lyric_test::CompileModule>
lyric_test::LyricTester::compileSingleModule(
    const std::string &code,
    const lyric_test::TesterOptions &options)
{
    lyric_test::LyricTester tester(options);
    auto status = tester.configure();
    if (!status.isOk())
        return status;
    auto runner = tester.m_runner;
    return runner->compileModuleInternal(code);
}

tempo_utils::Result<lyric_test::AnalyzeModule>
lyric_test::LyricTester::analyzeSingleModule(
    const std::string &code,
    const lyric_test::TesterOptions &options)
{
    lyric_test::LyricTester tester(options);
    auto status = tester.configure();
    if (!status.isOk())
        return status;
    auto runner = tester.m_runner;
    return runner->analyzeModuleInternal(code);
}

tempo_utils::Result<lyric_test::SymbolizeModule>
lyric_test::LyricTester::symbolizeSingleModule(
    const std::string &code,
    const lyric_test::TesterOptions &options)
{
    lyric_test::LyricTester tester(options);
    auto status = tester.configure();
    if (!status.isOk())
        return status;
    auto runner = tester.m_runner;
    return runner->symbolizeModuleInternal(code);
}

tempo_utils::Result<lyric_test::RunModule>
lyric_test::LyricTester::runSingleModule(
    const std::string &code,
    const lyric_test::TesterOptions &options)
{
    lyric_test::LyricTester tester(options);
    auto status = tester.configure();
    if (!status.isOk())
        return status;
    return tester.runModule(code);
}