#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_build/dependency_loader.h>
#include <lyric_test/matchers.h>
#include <lyric_test/test_inspector.h>
#include <tempo_test/result_matchers.h>

#include "base_compiler_fixture.h"

class CompileImport : public BaseCompilerFixture {};

TEST_F(CompileImport, EvaluateImportSingleRelativeLocation)
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

    auto execResult = runComputationSet(depStates, mainKey);
    ASSERT_THAT (execResult, tempo_test::IsResult());
    auto interpreterExit = execResult.getResult();

    ASSERT_THAT (interpreterExit.mainReturn, DataCellInt(42));
}

TEST_F(CompileImport, EvaluateImportMultipleRelativeLocations)
{
    absl::flat_hash_map<lyric_build::TaskKey,lyric_build::TaskState> depStates;

    auto compileDep1Result = m_tester->compileModule(R"(
        def Dep1(): Int {
            1
        }
    )", "dep1");

    ASSERT_THAT (compileDep1Result, tempo_test::IsResult());
    auto dep1 = compileDep1Result.getResult().getComputation();
    auto dep1Id = dep1.getId();
    lyric_build::TaskKey dep1Key(dep1Id.getDomain(), dep1Id.getId());
    depStates[dep1Key] = dep1.getState();

    auto compileDep2Result = m_tester->compileModule(R"(
        def Dep2(): Int {
            2
        }
    )", "dep2");

    ASSERT_THAT (compileDep2Result, tempo_test::IsResult());
    auto dep2 = compileDep2Result.getResult().getComputation();
    auto dep2Id = dep2.getId();
    lyric_build::TaskKey dep2Key(dep2Id.getDomain(), dep2Id.getId());
    depStates[dep2Key] = dep2.getState();

    auto compileDep3Result = m_tester->compileModule(R"(
        def Dep3(): Int {
            3
        }
    )", "dep3");

    ASSERT_THAT (compileDep3Result, tempo_test::IsResult());
    auto dep3 = compileDep3Result.getResult().getComputation();
    auto dep3Id = dep3.getId();
    lyric_build::TaskKey dep3Key(dep3Id.getDomain(), dep3Id.getId());
    depStates[dep3Key] = dep3.getState();

    auto compileMainResult = m_tester->compileModule(R"(
        import from "/dep1" ...
        import from "/dep2" ...
        import from "/dep3" ...
        Dep1() + Dep2() + Dep3()
    )");

    ASSERT_THAT (compileMainResult, tempo_test::IsResult());
    auto main = compileMainResult.getResult().getComputation();
    auto mainId = main.getId();
    lyric_build::TaskKey mainKey(mainId.getDomain(), mainId.getId());
    depStates[mainKey] = main.getState();

    auto execResult = runComputationSet(depStates, mainKey);
    ASSERT_THAT (execResult, tempo_test::IsResult());
    auto interpreterExit = execResult.getResult();

    ASSERT_THAT (interpreterExit.mainReturn, DataCellInt(6));
}
