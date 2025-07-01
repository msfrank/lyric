#include <iostream>
#include <lyric_build/artifact_loader.h>
#include <lyric_build/dependency_loader.h>

#include <lyric_test/lyric_protocol_tester.h>
#include <lyric_test/mock_binder.h>
#include <lyric_test/test_result.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/chain_loader.h>
#include <lyric_test/test_inspector.h>
#include <tempo_utils/directory_maker.h>

lyric_test::LyricProtocolTester::LyricProtocolTester(const ProtocolTesterOptions &options)
    : m_options(options)
{
    m_runner = TestRunner::create(
        options.testRootDirectory,
        options.useInMemoryCache,
        options.isTemporary,
        options.keepBuildOnUnexpectedResult,
        options.taskRegistry,
        options.bootstrapLoader,
        options.fallbackLoader,
        options.taskSettings);
}

tempo_utils::Status
lyric_test::LyricProtocolTester::configure()
{
    return m_runner->configureBaseTester();
}

const lyric_test::TestRunner *
lyric_test::LyricProtocolTester::getRunner() const
{
    return m_runner.get();
}

tempo_utils::Result<lyric_test::RunModule>
lyric_test::LyricProtocolTester::runModuleInMockSandbox(
    const std::string &code,
    const std::filesystem::path &modulePath,
    const std::filesystem::path &baseDir)
{
    if (!m_runner->isConfigured())
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "tester is unconfigured");

    // write the code to a module file in the src directory
    lyric_common::ModuleLocation moduleLocation;
    TU_ASSIGN_OR_RETURN (moduleLocation, m_runner->writeModuleInternal(code, modulePath, baseDir));

    lyric_build::TaskId target("compile_module", moduleLocation.toString());

    // compile the module file
    lyric_build::TargetComputationSet targetComputationSet;
    TU_ASSIGN_OR_RETURN (targetComputationSet, m_runner->computeTargetInternal(target));

    auto targetComputation = targetComputationSet.getTarget(target);
    TU_ASSERT (targetComputation.isValid());

    TU_CONSOLE_OUT << "";
    TU_CONSOLE_OUT << "======== RUN: " << moduleLocation << " ========";
    TU_CONSOLE_OUT << "";

    auto *builder = m_runner->getBuilder();
    auto cache = builder->getCache();
    auto tempRoot = builder->getTempRoot();

    // define the module origin
    auto origin = lyric_common::ModuleLocation::fromString(
        absl::StrCat("dev.zuri.tester://", tempo_utils::UUID::randomUUID().toString()));

    // construct the loader
    auto targetState = targetComputation.getState();
    lyric_build::BuildGeneration targetGen(targetState.getGeneration());
    lyric_build::TempDirectory tempDirectory(tempRoot, targetGen);
    std::shared_ptr<lyric_build::DependencyLoader> dependencyLoader;
    TU_ASSIGN_OR_RETURN (dependencyLoader, lyric_build::DependencyLoader::create(
        origin, targetComputation, cache, &tempDirectory));

    lyric_runtime::InterpreterStateOptions options;

    std::vector<std::shared_ptr<lyric_runtime::AbstractLoader>> loaderChain;
    loaderChain.push_back(builder->getBootstrapLoader());
    loaderChain.push_back(dependencyLoader);
    options.loader = std::make_shared<lyric_runtime::ChainLoader>(loaderChain);

    // construct the interpreter state
    std::shared_ptr<lyric_runtime::InterpreterState> state;
    TU_ASSIGN_OR_RETURN (state, lyric_runtime::InterpreterState::create(
        options, origin.resolve(moduleLocation)));

    // run the module in the interpreter
    TestInspector inspector;
    lyric_runtime::BytecodeInterpreter interp(state, &inspector);
    MockBinder mockBinder(m_options.protocolMocks);
    lyric_runtime::InterpreterExit exit;
    TU_ASSIGN_OR_RETURN (exit, mockBinder.run(&interp));

    // return the interpreter result
    return RunModule(m_runner, targetComputation, targetComputationSet.getDiagnostics(), state, exit);
}

tempo_utils::Result<lyric_test::RunModule>
lyric_test::LyricProtocolTester::runSingleModuleInMockSandbox(
    const std::string &code,
    const ProtocolTesterOptions &options)
{
    LyricProtocolTester tester(options);
    auto status = tester.configure();
    if (!status.isOk())
        return status;
    return tester.runModuleInMockSandbox(code);
}
