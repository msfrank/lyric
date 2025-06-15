#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "base_compiler_fixture.h"

class CompileLoadAndStoreMacro : public BaseCompilerFixture {};

// TEST_F(CompileLoadAndStoreMacro, EvaluateMacroLoadAndStore)
// {
//     auto result = m_tester->runModule(R"(
//         def add10(x: Int): Int {
//             var tmp: Int = 0
//             @{
//                 LoadData(10)
//                 PushResult(typeof Int)
//                 StoreData(tmp)
//             }
//             x + tmp
//         }
//         add10(5)
//     )");
//
//     ASSERT_THAT (result, tempo_test::ContainsResult(
//         RunModule(
//             DataCellInt(15))));
// }

TEST_F(CompileLoadAndStoreMacro, EvaluateMacroLoadAndStore)
{
    auto result = m_tester->runModule(R"(
        var tmp: Int = 0
        @{
            LoadData(10)
            StoreData(tmp)
        }
        tmp
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(10))));
}

