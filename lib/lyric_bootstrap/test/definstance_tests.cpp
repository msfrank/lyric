#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CoreDefinstance, EvaluateInstanceValMember)
{
    auto result = runModule(R"(
        definstance Foo {
            val i: Int = 100
        }
        Foo.i
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(100))));
}

TEST(CoreDefinstance, EvaluateInstanceVarMember)
{
    auto result = runModule(R"(
        definstance Foo {
            var i: Int = 100
        }
        Foo.i
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(100))));
}

TEST(CoreDefinstance, EvaluateInstanceMethod)
{
    auto result = runModule(R"(
        definstance Foo {
            def identity(x: Int): Int {
                x
            }
        }
        Foo.identity(42)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}

TEST(CoreDefinstance, EvaluateInstanceApplication)
{
    auto result = runModule(R"(
        definstance IntReverseOrdering {
            impl Ordered[Int] {
                def compare(lhs: Int, rhs: Int): Int {
                    cond {
                        when lhs > rhs     -1
                        when lhs < rhs      1
                        else                0
                    }
                }
            }
        }

        def max[T](x1: T, x2: T, using ord: Ordered[T]): T {
            if ord.compare(x1, x2) >= 0 then x1 else x2
        }

        max(1, 2, ord = IntReverseOrdering)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(1))));
}

TEST(CoreDefinstance, EvaluateUsingInstanceApplication)
{
    auto result = runModule(R"(
        definstance IntReverseOrdering {
            impl Ordered[Int] {
                def compare(lhs: Int, rhs: Int): Int {
                    cond {
                        when lhs > rhs     -1
                        when lhs < rhs      1
                        else                0
                    }
                }
            }
        }

        def max[T](x1: T, x2: T, using ord: Ordered[T]): T {
            if ord.compare(x1, x2) >= 0 then x1 else x2
        }

        using IntReverseOrdering

        max(1, 2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(1))));
}