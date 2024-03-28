#include <gtest/gtest.h>

#include <lyric_test/matchers.h>

#include "test_helpers.h"

TEST(CoreMap, TestEvaluateMapContainsNoEntries)
{
    auto result = runModule(R"(
        val names: Map = Map{}
        names.Contains("one")
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(false)))));
}

TEST(CoreMap, TestEvaluateMapContainsSingleEntry)
{
    auto result = runModule(R"(
        val names: Map = Map{ Pair{first = "one", second = 1} }
        names.Contains("one")
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}

TEST(CoreMap, TestEvaluateMapContainsMultipleEntries)
{
    auto result = runModule(R"(
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
    auto result = runModule(R"(
        val names: Map = Map{ Pair{first = "one", second = 1} }
        names.GetOrElse("one", 0)
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(1)))));
}

TEST(CoreMap, TestEvaluateMapGetMultipleEntries)
{
    auto result = runModule(R"(
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
    auto result = runModule(R"(
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
    auto result = runModule(R"(
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
    auto result = runModule(R"(
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

TEST(CoreMap, TestEvaluateMapIterateImpl)
{
    auto result = runModule(R"(
        val names: Map = Map{
            Pair{first = "one", second = 1},
            Pair{first = "two", second = 2},
            Pair{first = "three", second = 3}
            }
        var count: Int = 0
        for n: Any in names {
            set count += 1
        }
        count
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(3)))));
}
