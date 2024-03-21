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
        options.preludeLocation,
        options.packageMap,
        options.buildConfig,
        options.buildVendorConfig);
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
    const std::filesystem::path &path)
{
    if (!m_runner->isConfigured())
        return TestStatus::forCondition(TestCondition::kTestInvariant,
            "tester is unconfigured");

    // write the code to a module file in the src directory
    auto writeSourceResult = m_runner->writeModuleInternal(code, path);
    if (writeSourceResult.isStatus())
        return writeSourceResult.getStatus();
    auto sourcePath = writeSourceResult.getResult();
    lyric_build::TaskId target("compile_module", sourcePath);

    // compile the module file
    auto buildResult = m_runner->computeTargetInternal(target, {}, {}, {});
    if (buildResult.isStatus())
        return buildResult.getStatus();
    auto targetComputationSet = buildResult.getResult();
    auto targetComputation = targetComputationSet.getTarget(target);
    TU_ASSERT (targetComputation.isValid());

    // construct module location based on the source path
    std::filesystem::path modulePath = "/";
    modulePath /= sourcePath;
    modulePath.replace_extension();
    lyric_common::AssemblyLocation moduleLocation(modulePath);

    TU_CONSOLE_OUT << "";
    TU_CONSOLE_OUT << "======== RUN: " << moduleLocation << " ========";
    TU_CONSOLE_OUT << "";

    auto *builder = m_runner->getBuilder();
    auto cache = builder->getCache();

    // construct the loader
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
    loaderChain.push_back(builder->getPackageLoader());
    options.loader = std::make_shared<lyric_runtime::ChainLoader>(loaderChain);

    // construct the interpreter state
    auto createInterpreterStateResult = lyric_runtime::InterpreterState::create(options, moduleLocation);
    if (createInterpreterStateResult.isStatus())
        return createInterpreterStateResult.getStatus();
    auto state = createInterpreterStateResult.getResult();

    // run the module in the interpreter
    TestInspector inspector;
    lyric_runtime::BytecodeInterpreter interp(state, &inspector);
    MockBinder mockBinder(m_options.protocolMocks);
    auto binderRunResult = mockBinder.run(&interp);

    // return the interpreter result
    if (binderRunResult.isStatus())
        return binderRunResult.getStatus();
    return RunModule(m_runner, targetComputation,
        targetComputationSet.getDiagnostics(), binderRunResult.getResult());
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
