#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/spanset_matchers.h>

#include "base_compiler_fixture.h"

class CompileDefenum : public BaseCompilerFixture {};

TEST_F(CompileDefenum, EvaluateAbstractEnumDescriptor)
{
    auto result = m_tester->runModule(R"(
        defenum Direction {
            case North
            case South
            case East
            case West
        }
        #Direction
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        MatchesDescriptorSection(lyric_object::LinkageSection::Enum))));
}

TEST_F(CompileDefenum, EvaluateAbstractEnumFails)
{
    auto result = m_tester->compileModule(R"(
        defenum Direction {
            case North
            case South
            case East
            case West
        }
        Direction
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_compiler::CompilerCondition::kSyntaxError))));
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
        Direction.North
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellRef(lyric_common::SymbolPath({"Direction", "North"})))));
}

TEST_F(CompileDefenum, EvaluateEnumCaseVal)
{
    auto result = m_tester->runModule(R"(
        defenum Direction {
            val Index: Int
            init(index: Int) {
                this.Index = index
            }
            case North(1)
            case South(2)
            case East(3)
            case West(4)
        }
        Direction.North.Index
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(1))));
}

TEST_F(CompileDefenum, EvaluateEnumCaseDef)
{
    auto result = m_tester->runModule(R"(
        defenum Direction {
            val abbreviation: String
            init(abbreviation: String) {
                this.abbreviation = abbreviation
            }
            def IndexOf(): Int {
                cond {
                    when this.abbreviation == "N" -> 1
                    when this.abbreviation == "S" -> 2
                    when this.abbreviation == "E" -> 3
                    when this.abbreviation == "W" -> 4
                                             else -> -1
                }
            }
            case North("N")
            case South("S")
            case East("E")
            case West("W")
        }
        Direction.East.IndexOf()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}

TEST_F(CompileDefenum, EvaluateEnumCaseDefWithNoReturnType)
{
    auto result = m_tester->runModule(R"(
        defenum Direction {
            val abbreviation: String
            init(abbreviation: String) {
                this.abbreviation = abbreviation
            }
            def NoReturn() {
                42
            }
            case North("N")
            case South("S")
            case East("E")
            case West("W")
        }
        Direction.East.NoReturn()
    )");

    ASSERT_THAT (result,
        tempo_test::ContainsResult(RunModule(
            MatchesDataCellType(lyric_runtime::DataCellType::INVALID))));
}
