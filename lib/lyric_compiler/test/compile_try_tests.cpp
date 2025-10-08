#include <gtest/gtest.h>

#include <lyric_assembler/assembler_result.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_compiler_fixture.h"

class CompileTry : public BaseCompilerFixture {};

TEST_F(CompileTry, EvaluateTryCatch)
{
    auto result = m_tester->runModule(R"(
        var x: Int = 0
        try {
            raise Internal{message="something failed"}
        } catch {
            when ex: Internal
              set x = 1
        }
        x
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(1))));
}

