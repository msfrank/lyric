#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_bootstrap_fixture.h"

class MapTests : public BaseBootstrapFixture {};

TEST_F(MapTests, TestEvaluateMapContainsNoEntries)
{
    auto result = runModule(R"(
        val names: Map = Map{}
        names.Contains("one")
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST_F(MapTests, TestEvaluateMapContainsSingleEntry)
{
    auto result = runModule(R"(
        val names: Map = Map{ Pair{first = "one", second = 1} }
        names.Contains("one")
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(MapTests, TestEvaluateMapContainsMultipleEntries)
{
    auto result = runModule(R"(
        val names: Map = Map{
            Pair{first = "one", second = 1},
            Pair{first = "two", second = 2},
            Pair{first = "three", second = 3}
            }
        names.Contains("three")
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(MapTests, TestEvaluateMapGetSingleEntry)
{
    auto result = runModule(R"(
        val names: Map = Map{ Pair{first = "one", second = 1} }
        names.GetOrElse("one", 0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(1))));
}

TEST_F(MapTests, TestEvaluateMapGetMultipleEntries)
{
    auto result = runModule(R"(
        val names: Map = Map{
            Pair{first = "one", second = 1},
            Pair{first = "two", second = 2},
            Pair{first = "three", second = 3}
            }
        names.GetOrElse("three", 0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}

TEST_F(MapTests, TestEvaluateMapSizeMultipleEntries)
{
    auto result = runModule(R"(
        val names: Map = Map{
            Pair{first = "one", second = 1},
            Pair{first = "two", second = 2},
            Pair{first = "three", second = 3}
            }
        names.Size()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}

TEST_F(MapTests, TestEvaluateMapUpdate)
{
    auto result = runModule(R"(
        var names: Map = Map{
            Pair{first = "one", second = 1},
            Pair{first = "two", second = 2}
            }
        set names = names.Update("three", 3)
        names.GetOrElse("three", 0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}

TEST_F(MapTests, TestEvaluateMapRemove)
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

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST_F(MapTests, TestEvaluateMapIterateImpl)
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

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}
