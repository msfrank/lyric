#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "test_helpers.h"

TEST(CoreDefenum, EvaluateEnum)
{
    auto result = runModule(R"(
        defenum Direction {
            case North
            case South
            case East
            case West
        }
        Direction
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellRef(lyric_common::SymbolPath({"Direction"})))));
}

TEST(CoreDefenum, EvaluateEnumCase)
{
    auto result = runModule(R"(
        defenum Direction {
            case North
            case South
            case East
            case West
        }
        North
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellRef(lyric_common::SymbolPath({"North"})))));
}

TEST(CoreDefenum, EvaluateEnumCaseVal)
{
    auto result = runModule(R"(
        defenum Direction {
            val index: Int
            init(index: Int) {
                set this.index = index
            }
            case North(1)
            case South(2)
            case East(3)
            case West(4)
        }
        North.index
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(1))));
}

TEST(CoreDefenum, EvaluateEnumCaseDef)
{
    auto result = runModule(R"(
        defenum Direction {
            val abbreviation: String
            init(abbreviation: String) {
                set this.abbreviation = abbreviation
            }
            def indexOf(): Int {
                cond {
                    when this.abbreviation == "N"  1
                    when this.abbreviation == "S"  2
                    when this.abbreviation == "E"  3
                    when this.abbreviation == "W"  4
                    else     -1
                }
            }
            case North("N")
            case South("S")
            case East("E")
            case West("W")
        }
        East.indexOf()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}
