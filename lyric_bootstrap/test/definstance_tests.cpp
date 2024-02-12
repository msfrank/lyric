#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>

TEST(CoreDefinstance, EvaluateInstanceValMember)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        definstance Foo {
            val i: Int = 100
        }
        Foo.i
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(100)))));
}

TEST(CoreDefinstance, EvaluateInstanceVarMember)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        definstance Foo {
            var i: Int = 100
        }
        Foo.i
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(100)))));
}

TEST(CoreDefinstance, EvaluateInstanceMethod)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        definstance Foo {
            def identity(x: Int): Int {
                x
            }
        }
        Foo.identity(42)
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(42)))));
}

TEST(CoreDefinstance, EvaluateInstanceApplication)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        definstance IntReverseOrdering {
            impl Ordered[Int] {
                def compare(lhs: Int, rhs: Int): Int {
                    cond {
                        case lhs > rhs     -1
                        case lhs < rhs      1
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

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(1)))));
}

TEST(CoreDefinstance, EvaluateUsingInstanceApplication)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        definstance IntReverseOrdering {
            impl Ordered[Int] {
                def compare(lhs: Int, rhs: Int): Int {
                    cond {
                        case lhs > rhs     -1
                        case lhs < rhs      1
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

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(1)))));
}