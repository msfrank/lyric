#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_compiler_fixture.h"

class CompileConversion : public BaseCompilerFixture {};

TEST_F(CompileConversion, EvaluateValAssignmentConversion)
{
    auto result = m_tester->runModule(R"(
        def ReturnU8(): U8 { 42 as U8 }
        val foo: U16 = ReturnU8()
        foo
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandU16(42))));
}

TEST_F(CompileConversion, EvaluateVarAssignmentConversion)
{
    auto result = m_tester->runModule(R"(
        def ReturnU8(): U8 { 42 as U8 }
        var foo: U32 = ReturnU8()
        foo
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandU32(42))));
}
