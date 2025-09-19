#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_compiler_fixture.h"

class CompileDefinstance : public BaseCompilerFixture {};

TEST_F(CompileDefinstance, EvaluateInstanceWithExplicitInit)
{
    auto result = m_tester->runModule(R"(
        definstance Foo {
            val Index: Int
            init {
                set this.Index = 100
            }
        }
        Foo.Index
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(100))));
}

TEST_F(CompileDefinstance, EvaluateInstanceValMember)
{
    auto result = m_tester->runModule(R"(
        definstance Foo {
            val Index: Int = 100
        }
        Foo.Index
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(100))));
}

TEST_F(CompileDefinstance, EvaluateInstanceVarMember)
{
    auto result = m_tester->runModule(R"(
        definstance Foo {
            var Index: Int = 100
        }
        Foo.Index
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(100))));
}

TEST_F(CompileDefinstance, EvaluateInstanceMethod)
{
    auto result = m_tester->runModule(R"(
        definstance Foo {
            def Identity(x: Int): Int {
                x
            }
        }
        Foo.Identity(42)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}

TEST_F(CompileDefinstance, EvaluateInstanceMethodWithNoReturnType)
{
    auto result = m_tester->runModule(R"(
        definstance Foo {
            def NoReturn() {
                42
            }
        }
        Foo.NoReturn()
    )");

    ASSERT_THAT (result,
        tempo_test::ContainsResult(RunModule(
            MatchesDataCellType(lyric_runtime::DataCellType::INVALID))));
}

TEST_F(CompileDefinstance, EvaluateInstanceApplication)
{
    auto result = m_tester->runModule(R"(
        definstance IntReverseOrdering {
            impl Ordered[Int] {
                def Compare(lhs: Int, rhs: Int): Int {
                    cond {
                        when lhs > rhs     -1
                        when lhs < rhs      1
                        else                0
                    }
                }
            }
        }

        def max[T](x1: T, x2: T, using ord: Ordered[T]): T {
            ord.Compare(x1, x2) >= 0 then x1 else x2
        }

        max(1, 2, ord = IntReverseOrdering)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(1))));
}

TEST_F(CompileDefinstance, EvaluateUsingInstanceApplication)
{
    auto result = m_tester->runModule(R"(
        definstance IntReverseOrdering {
            impl Ordered[Int] {
                def Compare(lhs: Int, rhs: Int): Int {
                    cond {
                        when lhs > rhs     -1
                        when lhs < rhs      1
                        else                0
                    }
                }
            }
        }

        def max[T](x1: T, x2: T, using ord: Ordered[T]): T {
            ord.Compare(x1, x2) >= 0 then x1 else x2
        }

        using IntReverseOrdering

        max(1, 2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(1))));
}