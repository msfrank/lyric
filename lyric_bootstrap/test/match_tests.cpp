
#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>

TEST(CoreMatch, TestIsAEquals)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        Object{} ^? Object
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}

TEST(CoreMatch, TestIsASubtype)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        Object{} ^? Any
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}

TEST(CoreMatch, TestIsADisjointType)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        defclass Test {
            init() from Object() {}
        }
        Object{} ^? Test
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(false)))));
}

TEST(CoreMatch, TestMatchIntrinsic)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val x: Any = 42
        match x {
            case t0: Bool       0
            case t1: Char       1
            case t2: Int        2
            case t3: Float      3
        }
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(2)))));
}

TEST(CoreMatch, TestMatchClass)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        defclass Test1 {
            init() from Object() {}
        }
        defclass Test2 {
            init() from Object() {}
        }
        defclass Test3 {
            init() from Object() {}
        }

        val x: Any = Test3{}
        match x {
            case t1: Test1      1
            case t2: Test2      2
            case t3: Test3      3
        }
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(3)))));
}

TEST(CoreMatch, TestMatchEnum)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        defenum Direction {
            case North
            case South
            case East
            case West
        }

        val x: Any = West
        match x {
            case North
                1
            case South
                2
            case East
                3
            case West
                4
        }
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(4)))));
}

TEST(CoreMatch, TestMatchDerefAlias)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        defclass Test1 {
            val x: Int
            init(x: Int) from Object() {
                set this.x = x
            }
            def getX(): Int {
                this.x
            }
        }

        val x: Any = Test1{42}
        match x {
            case t1: Test1
                t1.getX()
        }
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(42)))));
}

TEST(CoreMatch, TestMatchUnwrapGenericClass)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val x: Tuple3[Int,Int,Int] = Tuple3[Int,Int,Int]{1, 2, 3}
        match x {
            case Tuple3[Int, Int, Int](t1: Int, t2: Int, t3: Int)
                t1 + t2 + t3
        }
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(6)))));
}