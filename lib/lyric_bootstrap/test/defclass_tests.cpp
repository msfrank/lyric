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
            var i: Int
            init(i: Int) from Object() {
                set this.i = i
            }
        }
        var foo: Foo = Foo{100}
        foo.i
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(100))));
}

TEST(CoreDefclass, EvaluateDerefPublicVarDefaultInitializedMember)
{
    auto result = runModule(R"(
        defclass Foo {
            var i: Int = 100
        }
        var foo: Foo = Foo{}
        foo.i
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     DataCellInt(100))));
}

TEST(CoreDefclass, EvaluateDerefThisProtectedVarMember)
{
    auto result = runModule(R"(
        defclass Foo {
            var _i: Int
            init(i: Int) from Object() {
                set this._i = i
            }
        }
        defclass Bar {
            init(i: Int) from Foo(i) {}
            def add(i: Int): Int {
                i + this._i
            }
        }
        var bar: Bar = Bar{100}
        bar.add(100)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     DataCellInt(200))));
}

TEST(CoreDefclass, EvaluateDerefProtectedVarMemberFails)
{
    auto result = compileModule(R"(
        defclass Foo {
            var _i: Int
            init(i: Int) from Object() {
                set this._i = i
            }
        }
        var foo: Foo = Foo{100}
        foo._i
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_assembler::AssemblerCondition::kInvalidAccess))));
}

TEST(CoreDefclass, EvaluateDerefThisPrivateVarMember)
{
    auto result = runModule(R"(
        defclass Foo {
            var __i: Int
            init(i: Int) from Object() {
                set this.__i = i
            }
            def _i(): Int {
                this.__i
            }
        }
        defclass Bar {
            init(i: Int) from Foo(i) {}
            def add(i: Int): Int {
                i + this._i()
            }
        }
        var bar: Bar = Bar{100}
        bar.add(100)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(200))));
}

TEST(CoreDefclass, EvaluateDerefPrivateVarMemberFails)
{
    auto result = compileModule(R"(
        defclass Foo {
            var __i: Int
            init(i: Int) from Object() {
                set this.__i = i
            }
        }
        var foo: Foo = Foo{100}
        foo.__i
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_assembler::AssemblerCondition::kInvalidAccess))));
}

TEST(CoreDefclass, EvaluateInvokeMethod)
{
    auto result = runModule(R"(
        defclass Foo {
            var _i: Int
            init(i: Int) from Object() {
                set this._i = i
            }
            def i(): Int {
                this._i
            }
        }
        var foo: Foo = Foo{100}
        foo.i()
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(DataCellInt(100))));
}

TEST(CoreDefclass, EvaluateDefGenericClass)
{
    auto result = runModule(R"(
        defclass Foo[A] {
            var _i: A
            init(i: A) from Object() {
                set this._i = i
            }
            def i(): A {
                this._i
            }
        }
        val foo: Foo[Int] = Foo[Int]{100}
        val i: Int = foo.i()
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
            var _i: Int
            init(i: Int) from Object() {
                set this._i = i
            }
            def i[T](t: T): Tuple2[T,Int] {
                Tuple2[T,Int]{t, this._i}
            }
        }
        var foo: Foo = Foo{100}
        val tuple: Tuple2[Int,Int] = foo.i[Int](42)
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
            def i[T](t: T): Tuple2[T,S] {
                Tuple2[T,S]{t, this._s}
            }
        }
        var foo: Foo[Int] = Foo[Int]{100}
        val tuple: Tuple2[Int,Int] = foo.i(42)
        tuple.t0 + tuple.t1
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(DataCellInt(142))));
}
