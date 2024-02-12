//#include <string>
//
//#include <gtest/gtest.h>
//
//#include <lyric_test/lyric_tester.h>
//#include <lyric_test/test_matchers.h>
//#include <tempo_utils/unicode.h>
//
//TEST(CoreUtf16, EvaluateUtf16Literal)
//{
//    lyric_test::LyricTester tester;
//
//    auto status = tester.configure();
//    ASSERT_TRUE (status.isOk());
//
//    auto result = tester.runModule(R"(
//        "Hello, world!"
//    )");
//
//    using namespace std::string_literals;
//    ASSERT_THAT (result, lyric_test::IsDataCellUtf16(tempo_utils::convert_to_utf16("Hello, world!")));
//}
//
//TEST(CoreUtf16, EvaluateIsEq)
//{
//    auto result = lyric_test::LyricTester::runSingleModule(R"(
//        "Hello" == "Hello"
//    )");
//
//    ASSERT_THAT (result, lyric_test::IsDataCellBool(true));
//}
//
//TEST(CoreUtf16, EvaluateIsLt)
//{
//    auto result = lyric_test::LyricTester::runSingleModule(R"(
//        "hello" < "goodbye"
//    )");
//
//    ASSERT_THAT (result, lyric_test::IsDataCellBool(false));
//}
//
//TEST(CoreUtf16, EvaluateIsGt)
//{
//    auto result = lyric_test::LyricTester::runSingleModule(R"(
//        "hello" > "goodbye"
//    )");
//
//    ASSERT_THAT (result, lyric_test::IsDataCellBool(true));
//}
//
//TEST(CoreUtf16, EvaluateIsLe)
//{
//    auto result = lyric_test::LyricTester::runSingleModule(R"(
//        "hello" <= "goodbye"
//    )");
//
//    ASSERT_THAT (result, lyric_test::IsDataCellBool(false));
//}
//
//TEST(CoreUtf16, EvaluateIsGe)
//{
//    auto result = lyric_test::LyricTester::runSingleModule(R"(
//        "hello" >= "goodbye"
//    )");
//
//    ASSERT_THAT (result, lyric_test::IsDataCellBool(true));
//}