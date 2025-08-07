
#include "base_compiler_fixture.h"

#include <lyric_build/dependency_loader.h>
#include <lyric_runtime/chain_loader.h>
#include <lyric_test/test_inspector.h>

void
BaseCompilerFixture::SetUp()
{
    lyric_test::TesterOptions testerOptions;
    testerOptions.bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>(LYRIC_BUILD_BOOTSTRAP_DIR);
    m_tester = std::make_unique<lyric_test::LyricTester>(testerOptions);
    TU_RAISE_IF_NOT_OK (m_tester->configure());
}

tempo_utils::Result<lyric_runtime::InterpreterExit>
BaseCompilerFixture::runComputationSet(
        absl::flat_hash_map<lyric_build::TaskKey,lyric_build::TaskState> &depStates,
        const lyric_build::TaskKey &mainKey)
{
    auto randomString = tempo_utils::UUID::randomUUID().toString();
    auto origin = lyric_common::ModuleLocation::fromString(
        absl::StrCat("tester://", randomString));

    auto runner = m_tester->getRunner();
    auto builder = runner->getBuilder();
    lyric_build::TempDirectory tempDirectory(runner->getTesterDirectory(), randomString);

    std::shared_ptr<lyric_build::DependencyLoader> dependencyLoader;
    TU_ASSIGN_OR_RETURN (dependencyLoader, lyric_build::DependencyLoader::create(
        origin, depStates, builder->getCache(), &tempDirectory));

    lyric_runtime::InterpreterStateOptions options;
    options.mainLocation = origin.resolve(lyric_common::ModuleLocation::fromString(mainKey.getId()));

    // construct the interpreter state
    std::shared_ptr<lyric_runtime::InterpreterState> state;
    TU_ASSIGN_OR_RETURN (state, lyric_runtime::InterpreterState::create(
        builder->getBootstrapLoader(), dependencyLoader, options));

    // run the module in the interpreter
    lyric_test::TestInspector inspector;
    lyric_runtime::BytecodeInterpreter interp(state, &inspector);
    return interp.run();
}
