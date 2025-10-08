#include <gtest/gtest.h>

#include <lyric_assembler/assembler_result.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_compiler_fixture.h"

class CompileTry : public BaseCompilerFixture {};

TEST_F(CompileTry, EvaluateTryCatch)
{
    auto result = m_tester->runModule(R"(
        var x: Int = 0
        try {
            raise Internal{message="something failed"}
        } catch {
            when ex: Internal
              set x = 1
        }
        x
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(1))));
}

TEST_F(CompileTry, EvaluateTryCatchPredicate)
{
    auto result = m_tester->runModule(R"(
        var x: String = ""
        try {
            raise Internal{message="something failed"}
        } catch {
            when ex: Internal
              set x = ex.GetMessage()
        }
        x
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellString("something failed"))));
}

TEST_F(CompileTry, EvaluateNestedTryCatchInner)
{
    auto result = m_tester->runModule(R"(
        var x: String = ""
        try {
            try {
                raise Internal{message="something failed"}
            } catch {
                when ex: Internal
                  set x = "inner"
            }
        } catch {
            when ex: Unknown
              set x = "outer"
        }
        x
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellString("inner"))));
}

TEST_F(CompileTry, EvaluateNestedTryCatchOuter)
{
    auto result = m_tester->runModule(R"(
        var x: String = ""
        try {
            try {
                raise Internal{message="something failed"}
            } catch {
                when ex: Unknown
                  set x = "inner"
            }
        } catch {
            when ex: Internal
              set x = "outer"
        }
        x
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellString("outer"))));
}
