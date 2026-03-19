#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_bootstrap_fixture.h"

class NamespaceTests : public BaseBootstrapFixture {};

TEST_F(NamespaceTests, EvaluateNamespace)
{
    auto result = runModule(R"(
        namespace Foo {}
        Foo
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        MatchesDataCellType(lyric_runtime::DataCellType::NAMESPACE))));
}
