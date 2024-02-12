#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>

TEST(CoreMap, TestEvaluateMapContainsNoEntries)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val names: Map = Map{}
        names.Contains("one")
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(false)))));
}

TEST(CoreMap, TestEvaluateMapContainsSingleEntry)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val names: Map = Map{ Pair{first = "one", second = 1} }
        names.Contains("one")
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}

TEST(CoreMap, TestEvaluateMapContainsMultipleEntries)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val names: Map = Map{
            Pair{first = "one", second = 1},
            Pair{first = "two", second = 2},
            Pair{first = "three", second = 3}
            }
        names.Contains("three")
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}

TEST(CoreMap, TestEvaluateMapGetSingleEntry)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val names: Map = Map{ Pair{first = "one", second = 1} }
        names.GetOrElse("one", 0)
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(1)))));
}

TEST(CoreMap, TestEvaluateMapGetMultipleEntries)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val names: Map = Map{
            Pair{first = "one", second = 1},
            Pair{first = "two", second = 2},
            Pair{first = "three", second = 3}
            }
        names.GetOrElse("three", 0)
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(3)))));
}

TEST(CoreMap, TestEvaluateMapSizeMultipleEntries)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val names: Map = Map{
            Pair{first = "one", second = 1},
            Pair{first = "two", second = 2},
            Pair{first = "three", second = 3}
            }
        names.Size()
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(3)))));
}

TEST(CoreMap, TestEvaluateMapUpdate)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        var names: Map = Map{
            Pair{first = "one", second = 1},
            Pair{first = "two", second = 2}
            }
        set names = names.Update("three", 3)
        names.GetOrElse("three", 0)
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(3)))));
}

TEST(CoreMap, TestEvaluateMapRemove)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        var names: Map = Map{
            Pair{first = "one", second = 1},
            Pair{first = "two", second = 2},
            Pair{first = "three", second = 3}
            }
        set names = names.Remove("three")
        names.Contains("three")
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(false)))));
}