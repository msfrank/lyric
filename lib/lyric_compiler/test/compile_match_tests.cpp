
#include <gtest/gtest.h>

#include <lyric_assembler/assembler_result.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_compiler_fixture.h"

class CompileMatch : public BaseCompilerFixture {};

TEST_F(CompileMatch, EvaluateMatchTypeEquals)
{
    auto result = m_tester->runModule(R"(
        match Object{} {
            when x: Object          true
            else                    false
        }
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(CompileMatch, EvaluateMatchSubtype)
{
    auto result = m_tester->runModule(R"(
        match Object{} {
            when x: Any             true
            else                    false
        }
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(CompileMatch, EvaluateNoMatchDisjointType)
{
    auto result = m_tester->runModule(R"(
        defclass Test {
        }
        match Object{} {
            when x: Test            true
            else                    false
        }
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST_F(CompileMatch, CompileIsABoundedPlaceholderTypeDisjointFails)
{
    auto result = m_tester->compileModule(R"(
        def generic[T](t: T): Bool where T < Int {
            match t {
                when x: Float       true
                else                false
            }
        }
        generic(42)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_compiler::CompilerCondition::kSyntaxError))));
}

TEST_F(CompileMatch, EvaluateMatchIntrinsic)
{
    auto result = m_tester->runModule(R"(
        val x: Any = 42
        match x {
            when t0: Bool       0
            when t1: Char       1
            when t2: Int        2
            when t3: Float      3
            else                nil
        }
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(2))));
}

TEST_F(CompileMatch, EvaluateMatchPlaceholderType)
{
    auto result = m_tester->runModule(R"(
        def generic[T](t: T): Any {
            match t {
                when i: Int     true
                else            false
            }
        }
        generic(42)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(CompileMatch, EvaluateNoMatchPlaceholderType)
{
    auto result = m_tester->runModule(R"(
        def generic[T](t: T): Any {
            match t {
                when f: Float   true
                else            false
            }
        }
        generic(42)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST_F(CompileMatch, EvaluateMatchBoundedPlaceholderType)
{
    auto result = m_tester->runModule(R"(
        def generic[T](t: T): Any where T < Intrinsic {
            match t {
                when i: Int     true
                else            false
            }
        }
        generic(42)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(CompileMatch, EvaluateNoMatchBoundedPlaceholderType)
{
    auto result = m_tester->runModule(R"(
        def generic[T](t: T): Any where T < Intrinsic {
            match t {
                when f: Float   true
                else            false
            }
        }
        generic(42)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST_F(CompileMatch, CompileMatchBoundedPlaceholderTypeDisjointFails)
{
    auto result = m_tester->compileModule(R"(
        def generic[T](t: T): Any where T < Int {
            match t {
                when f: Float   true
                else            false
            }
        }
        generic(42)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_compiler::CompilerCondition::kSyntaxError))));
}

TEST_F(CompileMatch, EvaluateMatchClass)
{
    auto result = m_tester->runModule(R"(
        defclass Test1 {
        }
        defclass Test2 {
        }
        defclass Test3 {
        }

        val x: Any = Test3{}
        match x {
            when t1: Test1      1
            when t2: Test2      2
            when t3: Test3      3
            else                nil
        }
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}

TEST_F(CompileMatch, EvaluateMatchEnum)
{
    auto result = m_tester->runModule(R"(
        defenum Direction {
            case North
            case South
            case East
            case West
        }

        val x: Any = West
        match x {
            when North          1
            when South          2
            when East           3
            when West           4
            else                nil
        }
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(4))));
}

TEST_F(CompileMatch, EvaluateMatchDerefAlias)
{
    auto result = m_tester->runModule(R"(
        defclass Test1 {
            val x: Int
            init(x: Int) {
                set this.x = x
            }
            def GetX(): Int {
                this.x
            }
        }

        val x: Any = Test1{42}
        match x {
            when t1: Test1      t1.GetX()
            else                nil
        }
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}

TEST_F(CompileMatch, EvaluateMatchUnwrapGenericClass)
{
    auto result = m_tester->runModule(R"(
        val x: Tuple3[Int,Int,Int] = Tuple3[Int,Int,Int]{1, 2, 3}
        match x {
            when Tuple3[Int, Int, Int](t1: Int, t2: Int, t3: Int)
                t1 + t2 + t3
        }
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(6))));
}