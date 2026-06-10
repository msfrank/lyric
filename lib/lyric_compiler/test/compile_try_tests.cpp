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
            when ex: Internal { x = 1 }
        }
        x
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandInt(1))));
}

TEST_F(CompileTry, EvaluateTryCatchPredicate)
{
    auto result = m_tester->runModule(R"(
        var x: String = ""
        try {
            raise Internal{message="something failed"}
        } catch {
            when ex: Internal { x = ex.GetMessage() }
        }
        x
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandString("something failed"))));
}

TEST_F(CompileTry, EvaluateNestedTryCatchInner)
{
    auto result = m_tester->runModule(R"(
        var x: String = ""
        try {
            try {
                raise Internal{message="something failed"}
            } catch {
                when ex: Internal { x = "inner" }
            }
        } catch {
            when ex: Unknown { x = "outer" }
        }
        x
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandString("inner"))));
}

TEST_F(CompileTry, EvaluateNestedTryCatchOuter)
{
    auto result = m_tester->runModule(R"(
        var x: String = ""
        try {
            try {
                raise Internal{message="something failed"}
            } catch {
                when ex: Unknown { x = "inner" }
            }
        } catch {
            when ex: Internal { x = "outer" }
        }
        x
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandString("outer"))));
}


TEST_F(CompileTry, EvaluateRaiseNewExceptionInCatch)
{
    auto result = m_tester->runModule(R"(
        var x: Int = 0
        try {
            x += 1
            try {
                x += 1
                raise Internal{message="something failed"}
            } catch {
                when ex: Internal {
                  x += 1
                  raise Aborted{message="aborted"}
                }
            }
        } catch {
            when ex: Aborted { x += 1 }
        }
        x
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandInt(4))));
}

TEST_F(CompileTry, EvaluateReraiseExceptionInCatch)
{
    auto result = m_tester->runModule(R"(
        var x: Int = 0
        try {
            x += 1
            try {
                x += 1
                raise Internal{message="something failed"}
            } catch {
                when ex: Internal {
                  x += 1
                  raise ex
                }
            }
        } catch {
            when ex: Internal { x += 1 }
        }
        x
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandInt(4))));
}