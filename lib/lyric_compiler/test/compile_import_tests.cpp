#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_build/dependency_loader.h>
#include <lyric_runtime/chain_loader.h>
#include <lyric_test/matchers.h>
#include <lyric_test/test_inspector.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/spanset_matchers.h>

#include "base_compiler_fixture.h"

class CompileImport : public BaseCompilerFixture {};

TEST_F(CompileImport, EvaluateImportRelativeLocation)
{
    absl::flat_hash_map<lyric_build::TaskKey,lyric_build::TaskState> depStates;

    auto compileDep1Result = m_tester->compileModule(R"(
        def Return42(): Int {
            42
        }
    )", "dep1");

    ASSERT_THAT (compileDep1Result, tempo_test::IsResult());
    auto dep1 = compileDep1Result.getResult().getComputation();
    auto dep1Id = dep1.getId();
    lyric_build::TaskKey dep1Key(dep1Id.getDomain(), dep1Id.getId());
    depStates[dep1Key] = dep1.getState();

    auto compileMainResult = m_tester->compileModule(R"(
        import from "/dep1" ...
        Return42()
    )");

    ASSERT_THAT (compileMainResult, tempo_test::IsResult());
    auto main = compileMainResult.getResult().getComputation();
    auto mainId = main.getId();
    lyric_build::TaskKey mainKey(mainId.getDomain(), mainId.getId());
    depStates[mainKey] = main.getState();

    auto randomString = tempo_utils::UUID::randomUUID().toString();
    auto origin = lyric_common::ModuleLocation::fromString(
        absl::StrCat("tester://", randomString));

    auto runner = m_tester->getRunner();
    auto builder = runner->getBuilder();
    lyric_build::TempDirectory tempDirectory(runner->getTesterDirectory(), randomString);

    auto createLoaderResult = lyric_build::DependencyLoader::create(
        origin, depStates, builder->getCache(), &tempDirectory);
    ASSERT_THAT (createLoaderResult, tempo_test::IsResult());
    auto dependencyLoader = createLoaderResult.getResult();

    lyric_runtime::InterpreterStateOptions options;

    std::vector<std::shared_ptr<lyric_runtime::AbstractLoader>> loaderChain;
    loaderChain.push_back(builder->getBootstrapLoader());
    loaderChain.push_back(dependencyLoader);
    options.loader = std::make_shared<lyric_runtime::ChainLoader>(loaderChain);

    // construct the interpreter state
    auto mainLocation = origin.resolve(lyric_common::ModuleLocation::fromString(mainId.getId()));
    auto createStateResult = lyric_runtime::InterpreterState::create(options, mainLocation);
    ASSERT_THAT (createStateResult, tempo_test::IsResult());
    auto state = createStateResult.getResult();

    // run the module in the interpreter
    lyric_test::TestInspector inspector;
    lyric_runtime::BytecodeInterpreter interp(state, &inspector);
    auto execResult = interp.run();
    ASSERT_THAT (execResult, tempo_test::IsResult());
    auto interpreterExit = execResult.getResult();

    ASSERT_THAT (interpreterExit.mainReturn, DataCellInt(42));
}
