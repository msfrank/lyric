#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_compiler_fixture.h"

class CompileVal : public BaseCompilerFixture {};

TEST_F(CompileVal, EvaluateVal)
{
    auto result = m_tester->runModule(R"(
        val foo: Int = 100
        foo
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(100))));
}