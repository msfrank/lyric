#include <gtest/gtest.h>

#include <lyric_assembler/assembler_result.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CoreDefclass, EvaluateNewInstanceWithDefaultConstructor)
{
    auto result = runModule(R"(
        defclass Foo {
            val value: Int = 42
        }
        Foo{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST(CoreDefclass, EvaluateNewInstanceFromDefaultSuperclass)
{
    auto result = runModule(R"(
        defclass Foo {
            init() {}
        }
        Foo{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST(CoreDefclass, EvaluateNewInstanceFromSuperclass)
{
    auto result = runModule(R"(
        defclass Foo {
            init() from Object() {}
        }
        Foo{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST(CoreDefclass, EvaluateNewInstanceWithDefaultInitializedMember)
{
    auto result = runModule(R"(
        defclass Foo {
            val i: Int = 100
            init() {}
        }
        Foo{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST(CoreDefclass, EvaluateNewInstanceWithDefaultConstructorAndDefaultInitializedMember)
{
    auto result = runModule(R"(
        defclass Foo {
            val i: Int = 100
        }
        Foo{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST(CoreDefclass, EvaluateDerefPublicVarMember)
{
    auto result = runModule(R"(
        defclass Foo {
            var Index: Int
            init(i: Int) from Object() {
                set this.Index = i
            }
        }
        var foo: Foo = Foo{100}
        foo.Index
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(100))));
}

TEST(CoreDefclass, EvaluateDerefPublicVarDefaultInitializedMember)
{
    auto result = runModule(R"(
        defclass Foo {
            var Index: Int = 100
        }
        var foo: Foo = Foo{}
        foo.Index
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     DataCellInt(100))));
}

TEST(CoreDefclass, EvaluateDerefThisProtectedVarMember)
{
    auto result = runModule(R"(
        defclass Foo {
            var index: Int
            init(i: Int) from Object() {
                set this.index = i
            }
        }
        defclass Bar {
            init(i: Int) from Foo(i) {}
            def Add(i: Int): Int {
                i + this.index
            }
        }
        var bar: Bar = Bar{100}
        bar.Add(100)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     DataCellInt(200))));
}

TEST(CoreDefclass, EvaluateDerefProtectedVarMemberFails)
{
    auto result = compileModule(R"(
        defclass Foo {
            var index: Int
            init(i: Int) from Object() {
                set this.index = i
            }
        }
        var foo: Foo = Foo{100}
        foo.index
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_assembler::AssemblerCondition::kInvalidAccess))));
}

TEST(CoreDefclass, EvaluateDerefThisPrivateVarMember)
{
    auto result = runModule(R"(
        defclass Foo {
            var _index: Int
            init(i: Int) from Object() {
                set this._index = i
            }
            def index(): Int {
                this._index
            }
        }
        defclass Bar {
            init(i: Int) from Foo(i) {}
            def Add(i: Int): Int {
                i + this.index()
            }
        }
        var bar: Bar = Bar{100}
        bar.Add(100)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(200))));
}

TEST(CoreDefclass, EvaluateDerefPrivateVarMemberFails)
{
    auto result = compileModule(R"(
        defclass Foo {
            var _index: Int
            init(i: Int) from Object() {
                set this._index = i
            }
        }
        var foo: Foo = Foo{100}
        foo._index
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_assembler::AssemblerCondition::kInvalidAccess))));
}

TEST(CoreDefclass, EvaluateInvokeMethod)
{
    auto result = runModule(R"(
        defclass Foo {
            var _index: Int
            init(i: Int) from Object() {
                set this._index = i
            }
            def Index(): Int {
                this._index
            }
        }
        var foo: Foo = Foo{100}
        foo.Index()
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(DataCellInt(100))));
}

TEST(CoreDefclass, EvaluateDefGenericClass)
{
    auto result = runModule(R"(
        defclass Foo[A] {
            var _index: A
            init(i: A) from Object() {
                set this._index = i
            }
            def Index(): A {
                this._index
            }
        }
        val foo: Foo[Int] = Foo[Int]{100}
        val i: Int = foo.Index()
        i
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     DataCellInt(100))));
}

TEST(CoreDefclass, EvaluateInvokeGenericMethod)
{
    auto result = runModule(R"(
        defclass Foo {
            var _index: Int
            init(i: Int) from Object() {
                set this._index = i
            }
            def Tuple[T](t: T): Tuple2[T,Int] {
                Tuple2[T,Int]{t, this._index}
            }
        }
        var foo: Foo = Foo{100}
        val tuple: Tuple2[Int,Int] = foo.Tuple[Int](42)
        tuple.t0 + tuple.t1
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(DataCellInt(142))));
}

TEST(CoreDefclass, EvaluateInvokeGenericMethodForGenericClass)
{
    auto result = runModule(R"(
        defclass Foo[S] {
            var _s: S
            init(s: S) from Object() {
                set this._s = s
            }
            def Tuple[T](t: T): Tuple2[T,S] {
                Tuple2[T,S]{t, this._s}
            }
        }
        var foo: Foo[Int] = Foo[Int]{100}
        val tuple: Tuple2[Int,Int] = foo.Tuple(42)
        tuple.t0 + tuple.t1
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(DataCellInt(142))));
}

TEST(CoreDefclass, EvaluateNewInstanceOfSealedClass)
{
    auto result = runModule(R"(
        defclass Foo sealed {
            val value: Int = 42
        }
        Foo{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST(CoreDefclass, EvaluateDefineSubclassOfFinalClassFails)
{
    auto result = compileModule(R"(
        defclass Foo final {
        }
        defclass Bar {
            init() from Foo() {}
        }
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_assembler::AssemblerCondition::kInvalidAccess))));
}

TEST(CoreDefclass, EvaluateNewInstanceOfFinalClass)
{
    auto result = runModule(R"(
        defclass Foo final {
            val value: Int = 42
        }
        Foo{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}
