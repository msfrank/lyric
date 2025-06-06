#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "base_compiler_fixture.h"

class CompileDefenum : public BaseCompilerFixture {};

TEST_F(CompileDefenum, EvaluateEnum)
{
    auto result = m_tester->runModule(R"(
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

TEST_F(CompileDefenum, EvaluateEnumCase)
{
    auto result = m_tester->runModule(R"(
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

TEST_F(CompileDefenum, EvaluateEnumCaseVal)
{
    auto result = m_tester->runModule(R"(
        defenum Direction {
            val Index: Int
            init(index: Int) {
                set this.Index = index
            }
            case North(1)
            case South(2)
            case East(3)
            case West(4)
        }
        North.Index
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(1))));
}

TEST_F(CompileDefenum, EvaluateEnumCaseDef)
{
    auto result = m_tester->runModule(R"(
        defenum Direction {
            val abbreviation: String
            init(abbreviation: String) {
                set this.abbreviation = abbreviation
            }
            def IndexOf(): Int {
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
        East.IndexOf()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}

TEST_F(CompileDefenum, EvaluateEnumCaseDefWithNoReturnType)
{
    auto result = m_tester->runModule(R"(
        defenum Direction {
            val abbreviation: String
            init(abbreviation: String) {
                set this.abbreviation = abbreviation
            }
            def NoReturn() {
                42
            }
            case North("N")
            case South("S")
            case East("E")
            case West("W")
        }
        East.NoReturn()
    )");

    ASSERT_THAT (result,
        tempo_test::ContainsResult(RunModule(
            MatchesDataCellType(lyric_runtime::DataCellType::INVALID))));
}
