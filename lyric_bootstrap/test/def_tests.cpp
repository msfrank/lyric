#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>

TEST(CoreDef, EvaluateDefUnaryFunction)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        def add10(x: Int): Int {
            x + 10
        }
        add10(5)
    )");

    ASSERT_THAT (result, ContainsResult(
        RunModule(
            Return(DataCellInt(15)))));
}

TEST(CoreDef, EvaluateDefBinaryFunction)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        def subtractInts(x: Int, y: Int): Int {
            x - y
        }
        subtractInts(5, 4)
    )");

    ASSERT_THAT (result, ContainsResult(
        RunModule(
            Return(DataCellInt(1)))));
}

TEST(CoreDef, EvaluateDefFunctionWithNamedParams)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        def subtractInts(x: Int, named y: Int): Int {
            x - y
        }
        subtractInts(5, y = 4)
    )");

    ASSERT_THAT (result, ContainsResult(
        RunModule(
            Return(DataCellInt(1)))));
}

TEST(CoreDef, EvaluateDefFunctionWithDefaultInitializer)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        def subtractInts(x: Int, named y: Int = 1): Int {
            x - y
        }
        subtractInts(5)
    )");

    ASSERT_THAT (result, ContainsResult(
        RunModule(
            Return(DataCellInt(4)))));
}

//TEST(CoreDef, EvaluateDefFunctionWithVariadicParam)
//{
//    QByteArray code = R"(
//
//    def countArgs(x: Int, ... ): Int
//        1 + rest_size()
//    end
//    countArgs(5, 4, 3, 2, 1)
//    )";
//
//    CompilerOptions options;
//    options.bootDirectoryPath = LYRIC_OUTPUT_LIB_BOOT_DIR;
//    options.distributionLibDirectoryPath = LYRIC_OUTPUT_LIB_DIST_DIR;
//    LyricCompiler compiler(options);
//    auto compileResult = compiler.compile(code);
//    LOG_DEBUG << "compilation result:" << compileResult;
//    ASSERT_TRUE (compileResult.isResult());
//
//    BytecodeInterpreter interp(compileResult.getResult());
//
//    auto result = interp.invokeMain();
//    LOG_DEBUG << "result:" << result;
//    ASSERT_TRUE (result.isResult());
//    auto value = result.getResult();
//    ASSERT_TRUE (value.type == DataCellType::I64);
//    ASSERT_EQ (value.data.i64, 5);
//}

TEST(CoreDef, EvaluateDefGenericFunction)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        def identity[A](x: A): A {
            x
        }
        val sum: Int = 5 + identity(5)
        sum
    )");

    ASSERT_THAT (result, ContainsResult(
        RunModule(
            Return(DataCellInt(10)))));
}

TEST(CoreDef, EvaluateDefGenericFunctionWithUpperBound)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        def identity[A](x: A): A where A extends Int {
            x
        }
        identity(5)
    )");

    ASSERT_THAT (result, ContainsResult(
        RunModule(
            Return(DataCellInt(5)))));
}

TEST(CoreDef, EvaluateDefGenericFunctionWithCtxParameter)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        def sum[A](x1: A, x2: A, using math: Arithmetic[A, A]): A {
            math.add(x1, x2)
        }
        sum(5, 5)
    )");

    ASSERT_THAT (result, ContainsResult(
        RunModule(
            Return(DataCellInt(10)))));
}