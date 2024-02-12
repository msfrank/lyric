#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>

TEST(CoreDefconcept, EvaluateDefconcept)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        defconcept Sum[T] {
            def sum(x1: T, x2: T): T
        }
        #Sum
    )");

    ASSERT_THAT (result,
                 ContainsResult(RunModule(
                     Return(MatchesDescriptorSection(lyric_object::LinkageSection::Concept)))));
}

TEST(CoreDefconcept, EvaluateDefconceptImplementation)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
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
                 ContainsResult(
                     RunModule(Return(DataCellInt(5)))));
}

TEST(CoreDefconcept, EvaluateDefconceptAction)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
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
                 ContainsResult(
                     RunModule(Return(DataCellInt(5)))));
}
