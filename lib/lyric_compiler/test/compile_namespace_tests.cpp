#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_compiler_fixture.h"

class CompileNamespace : public BaseCompilerFixture {};

TEST_F(CompileNamespace, EvaluateDeclareNamespace)
{
    auto result = m_tester->runModule(R"(
        namespace foo {
        }
        #foo
    )");

    auto state = result.getResult().getInterpreterState().lock();

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        MatchesDescriptorSection(lyric_object::LinkageSection::Namespace))));
}

TEST_F(CompileNamespace, EvaluateDereferenceNamespacedGlobal)
{
    auto result = m_tester->runModule(R"(
        namespace foo {
            global val qux: Int = 42
        }
        foo.qux
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}

TEST_F(CompileNamespace, EvaluateInvokeNamespacedFunction)
{
    auto result = m_tester->runModule(R"(
        namespace foo {
            def bar(): Int { 42 }
        }
        foo.bar()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}
