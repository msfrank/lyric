
#include <gtest/gtest.h>

#include <lyric_assembler/assembler_result.h>
#include <lyric_test/matchers.h>

#include "test_helpers.h"

TEST(CoreMatch, TestIsAEquals)
{
    auto result = runModule(R"(
        Object{} ^? Object
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}

TEST(CoreMatch, TestIsASubtype)
{
    auto result = runModule(R"(
        Object{} ^? Any
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}

TEST(CoreMatch, TestIsADisjointType)
{
    auto result = runModule(R"(
        defclass Test {
            init() from Object() {}
        }
        Object{} ^? Test
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(false)))));
}

TEST(CoreMatch, TestIsAPlaceholderType)
{
    auto result = runModule(R"(
        def generic[T](t: T): Bool {
            t ^? Int
        }
        generic(42)
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}

TEST(CoreMatch, TestIsAPlaceholderTypeNoMatch)
{
    auto result = runModule(R"(
        def generic[T](t: T): Bool {
            t ^? Float
        }
        generic(42)
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(false)))));
}

TEST(CoreMatch, TestIsABoundedPlaceholderType)
{
    auto result = runModule(R"(
        def generic[T](t: T): Bool where T extends Intrinsic {
            t ^? Int
        }
        generic(42)
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}

TEST(CoreMatch, TestIsABoundedPlaceholderTypeNoMatch)
{
    auto result = runModule(R"(
        def generic[T](t: T): Bool where T extends Intrinsic {
            t ^? Float
        }
        generic(42)
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(false)))));
}

TEST(CoreMatch, TestIsABoundedPlaceholderTypeDisjoint)
{
    auto result = compileModule(R"(
        def generic[T](t: T): Bool where T extends Int {
            t ^? Float
        }
        generic(42)
    )");

    ASSERT_THAT (result, ContainsResult(
        CompileModule(
            SpansetContainsError(lyric_assembler::AssemblerCondition::kSyntaxError))));
}

TEST(CoreMatch, TestMatchIntrinsic)
{
    auto result = runModule(R"(
        val x: Any = 42
        match x {
            case t0: Bool       0
            case t1: Char       1
            case t2: Int        2
            case t3: Float      3
        }
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(2)))));
}

TEST(CoreMatch, TestMatchPlaceholderType)
{
    auto result = runModule(R"(
        def generic[T](t: T): Any {
            match t {
                case i: Int     true
                else            false
            }
        }
        generic(42)
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}

TEST(CoreMatch, TestMatchPlaceholderTypeNoMatch)
{
    auto result = runModule(R"(
        def generic[T](t: T): Any {
            match t {
                case f: Float   true
                else            false
            }
        }
        generic(42)
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(false)))));
}

TEST(CoreMatch, TestMatchBoundedPlaceholderType)
{
    auto result = runModule(R"(
        def generic[T](t: T): Any where T extends Intrinsic {
            match t {
                case i: Int     true
                else            false
            }
        }
        generic(42)
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}

TEST(CoreMatch, TestMatchBoundedPlaceholderTypeNoMatch)
{
    auto result = runModule(R"(
        def generic[T](t: T): Any where T extends Intrinsic {
            match t {
                case f: Float   true
                else            false
            }
        }
        generic(42)
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(false)))));
}

TEST(CoreMatch, TestMatchBoundedPlaceholderTypeDisjoint)
{
    auto result = compileModule(R"(
        def generic[T](t: T): Any where T extends Int {
            match t {
                case f: Float   true
                else            false
            }
        }
        generic(42)
    )");

    ASSERT_THAT (result, ContainsResult(
        CompileModule(
            SpansetContainsError(lyric_assembler::AssemblerCondition::kSyntaxError))));
}

TEST(CoreMatch, TestMatchClass)
{
    auto result = runModule(R"(
        defclass Test1 {
            init() from Object() {}
        }
        defclass Test2 {
            init() from Object() {}
        }
        defclass Test3 {
            init() from Object() {}
        }

        val x: Any = Test3{}
        match x {
            case t1: Test1      1
            case t2: Test2      2
            case t3: Test3      3
        }
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(3)))));
}

TEST(CoreMatch, TestMatchEnum)
{
    auto result = runModule(R"(
        defenum Direction {
            case North
            case South
            case East
            case West
        }

        val x: Any = West
        match x {
            case North
                1
            case South
                2
            case East
                3
            case West
                4
        }
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(4)))));
}

TEST(CoreMatch, TestMatchDerefAlias)
{
    auto result = runModule(R"(
        defclass Test1 {
            val x: Int
            init(x: Int) from Object() {
                set this.x = x
            }
            def getX(): Int {
                this.x
            }
        }

        val x: Any = Test1{42}
        match x {
            case t1: Test1
                t1.getX()
        }
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(42)))));
}

TEST(CoreMatch, TestMatchUnwrapGenericClass)
{
    auto result = runModule(R"(
        val x: Tuple3[Int,Int,Int] = Tuple3[Int,Int,Int]{1, 2, 3}
        match x {
            case Tuple3[Int, Int, Int](t1: Int, t2: Int, t3: Int)
                t1 + t2 + t3
        }
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(6)))));
}