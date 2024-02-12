#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>

TEST(CoreDefenum, EvaluateEnum)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        defenum Direction {
            case North
            case South
            case East
            case West
        }
        Direction
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(IsRefType(lyric_common::SymbolPath({"Direction"}))))));
}

TEST(CoreDefenum, EvaluateEnumCase)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        defenum Direction {
            case North
            case South
            case East
            case West
        }
        North
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(IsRefType(lyric_common::SymbolPath({"North"}))))));
}

TEST(CoreDefenum, EvaluateEnumCaseVal)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
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

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(1)))));
}

TEST(CoreDefenum, EvaluateEnumCaseDef)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        defenum Direction {
            val abbreviation: String
            init(abbreviation: String) {
                set this.abbreviation = abbreviation
            }
            def indexOf(): Int {
                cond {
                    case this.abbreviation == "N"  1
                    case this.abbreviation == "S"  2
                    case this.abbreviation == "E"  3
                    case this.abbreviation == "W"  4
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

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(3)))));
}
