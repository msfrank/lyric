#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_compiler_fixture.h"
#include "lyric_bootstrap/bootstrap_helpers.h"

class CompileCast : public BaseCompilerFixture {};

TEST_F(CompileCast, EvaluateUpcastWithSubtype)
{
    auto result = m_tester->runModule(R"(
        val foo: Int = 100
        val any = foo as Any
        any
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(100))));
}

TEST_F(CompileCast, EvaluateUpcastWithEqualType)
{
    auto result = m_tester->runModule(R"(
        val foo: Int = 100
        val any = foo as Int
        any
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(100))));
}

TEST_F(CompileCast, EvaluateUpcastWithSuperTypeFails)
{
    auto result = m_tester->compileModule(R"(
        val foo: Any = 100
        val int = foo as Int
        any
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(CompileModule(
        tempo_test::SpansetContainsError(
            lyric_compiler::CompilerCondition::kIncompatibleType))));
}

TEST_F(CompileCast, EvaluateUpcastWithDisjointTypeFails)
{
    auto result = m_tester->compileModule(R"(
        val foo: Int = 100
        val any = foo as Float
        any
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(CompileModule(
        tempo_test::SpansetContainsError(
            lyric_compiler::CompilerCondition::kIncompatibleType))));
}

TEST_F(CompileCast, EvaluateImplCastWithImplementedConcept)
{
    auto result = m_tester->runModule(R"(
        val eq = IntInstance as Equality[Int,Int]
        eq
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellRef(
            lyric_bootstrap::preludeSymbol("IntInstance")))));
}

TEST_F(CompileCast, EvaluateImplCastWithMultipleImplementedConcepts)
{
    auto result = m_tester->runModule(R"(
        val eq = IntInstance as Equality[Int,Int] & Arithmetic[Int]
        eq
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellRef(
            lyric_bootstrap::preludeSymbol("IntInstance")))));
}

TEST_F(CompileCast, EvaluateImplCastWithUnimplementedConceptFails)
{
    auto result = m_tester->compileModule(R"(
        val eq = IntInstance as Equality[Float,Float]
        eq
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(CompileModule(
        tempo_test::SpansetContainsError(
            lyric_compiler::CompilerCondition::kIncompatibleType))));
}
