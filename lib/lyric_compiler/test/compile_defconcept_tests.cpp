#include <gtest/gtest.h>

#include <lyric_assembler/assembler_result.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_compiler_fixture.h"

class CompileDefconcept : public BaseCompilerFixture {};

TEST_F(CompileDefconcept, EvaluateDefconcept)
{
    auto result = m_tester->runModule(R"(
        defconcept Sum[T] {
            decl sum(x1: T, x2: T): T
        }
        #Sum
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     MatchesDescriptorSection(lyric_object::LinkageSection::Concept))));
}

TEST_F(CompileDefconcept, EvaluateDefconceptImplementation)
{
    auto result = m_tester->runModule(R"(
        defconcept Sum[T] {
            decl sum(x1: T, x2: T): T
        }
        definstance SumInstance {
            impl Sum[Int] {
                def sum(x1: Int, x2: Int): Int {
                    x1 + x2
                }
            }
        }
        using SumInstance
        def sum[T](x1: T, x2: T, using sum: Sum[T]): T {
            sum.sum(x1, x2)
        }

        sum(1, 4)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(5))));
}

TEST_F(CompileDefconcept, EvaluateDefconceptAction)
{
    auto result = m_tester->runModule(R"(
        defconcept Sum[T] {
            decl sum(x1: T, x2: T): T
        }
        definstance SumInstance {
            impl Sum[Int] {
                def sum(x1: Int, x2: Int): Int {
                    x1 + x2
                }
            }
        }

        val concept: Sum[Int] = SumInstance

        concept.sum(1, 4)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(5))));
}

TEST_F(CompileDefconcept, EvaluateImplementationOfSealedConcept)
{
    auto result = m_tester->runModule(R"(
        defconcept Sum[T] sealed {
            decl sum(x1: T, x2: T): T
        }
        definstance SumInstance {
            impl Sum[Int] {
                def sum(x1: Int, x2: Int): Int {
                    x1 + x2
                }
            }
        }

        val concept: Sum[Int] = SumInstance

        concept.sum(1, 4)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(5))));
}

TEST_F(CompileDefconcept, EvaluateImplementationOfFinalConcept)
{
    auto result = m_tester->runModule(R"(
        defconcept Sum[T] final {
            decl sum(x1: T, x2: T): T
        }
        definstance SumInstance {
            impl Sum[Int] {
                def sum(x1: Int, x2: Int): Int {
                    x1 + x2
                }
            }
        }

        val concept: Sum[Int] = SumInstance

        concept.sum(1, 4)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(5))));
}
