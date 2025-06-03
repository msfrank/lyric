#include <gtest/gtest.h>

#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_compiler_fixture.h"

class CompileNew : public BaseCompilerFixture {};

TEST_F(CompileNew, EvaluateNewObject)
{
    auto result = m_tester->runModule(R"(
        Object{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellRef(lyric_bootstrap::preludeSymbol("Object")))));
}