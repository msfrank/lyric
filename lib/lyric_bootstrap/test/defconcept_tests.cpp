#include <gtest/gtest.h>

#include <lyric_assembler/assembler_result.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CoreDefconcept, EvaluateDefconcept)
{
    auto result = runModule(R"(
        defconcept Sum[T] {
            def sum(x1: T, x2: T): T
        }
        #Sum
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     MatchesDescriptorSection(lyric_object::LinkageSection::Concept))));
}

TEST(CoreDefconcept, EvaluateDefconceptImplementation)
{
    auto result = runModule(R"(
        defconcept Sum[T] {
            def sum(x1: T, x2: T): T
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

TEST(CoreDefconcept, EvaluateDefconceptAction)
{
    auto result = runModule(R"(
        defconcept Sum[T] {
            def sum(x1: T, x2: T): T
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

TEST(CoreDefconcept, EvaluateImplementationOfSealedConcept)
{
    auto result = runModule(R"(
        defconcept Sum[T] sealed {
            def sum(x1: T, x2: T): T
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

TEST(CoreDefconcept, EvaluateImplementationOfFinalConcept)
{
    auto result = runModule(R"(
        defconcept Sum[T] final {
            def sum(x1: T, x2: T): T
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
