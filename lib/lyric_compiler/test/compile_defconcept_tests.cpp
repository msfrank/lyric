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
            impl Sum[I64] {
                def sum(x1: I64, x2: I64): I64 {
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
                     RunModule(OperandInt(5))));
}

TEST_F(CompileDefconcept, EvaluateDefconceptAction)
{
    auto result = m_tester->runModule(R"(
        defconcept Sum[T] {
            decl sum(x1: T, x2: T): T
        }
        definstance SumInstance {
            impl Sum[I64] {
                def sum(x1: I64, x2: I64): I64 {
                    x1 + x2
                }
            }
        }

        val concept: Sum[I64] = SumInstance

        concept.sum(1, 4)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(OperandInt(5))));
}

TEST_F(CompileDefconcept, EvaluateImplementationOfSealedConcept)
{
    auto result = m_tester->runModule(R"(
        defconcept Sum[T] sealed {
            decl sum(x1: T, x2: T): T
        }
        definstance SumInstance {
            impl Sum[I64] {
                def sum(x1: I64, x2: I64): I64 {
                    x1 + x2
                }
            }
        }

        val concept: Sum[I64] = SumInstance

        concept.sum(1, 4)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(OperandInt(5))));
}

TEST_F(CompileDefconcept, EvaluateImplementationOfFinalConcept)
{
    auto result = m_tester->runModule(R"(
        defconcept Sum[T] final {
            decl sum(x1: T, x2: T): T
        }
        definstance SumInstance {
            impl Sum[I64] {
                def sum(x1: I64, x2: I64): I64 {
                    x1 + x2
                }
            }
        }

        val concept: Sum[I64] = SumInstance

        concept.sum(1, 4)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(OperandInt(5))));
}

TEST_F(CompileDefconcept, EvaluateDerefGlobalMember)
{
    auto result = m_tester->runModule(R"(
        defconcept Foo {
            global {
                val GlobalValue: I64 = 42
            }
        }
        Foo.GlobalValue
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandInt(42))));
}

TEST_F(CompileDefconcept, EvaluateInvokeGlobalMethod)
{
    auto result = m_tester->runModule(R"(
        defconcept Foo {
            global {
                def GetValue(): I64 { 42 }
            }
        }
        Foo.GetValue()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandInt(42))));
}
