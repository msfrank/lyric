#include <gtest/gtest.h>

#include <lyric_assembler/assembler_result.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_compiler_fixture.h"

class CompileDefclass : public BaseCompilerFixture {};

TEST_F(CompileDefclass, EvaluateNewInstanceWithDefaultConstructor)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
            val value: Int = 42
        }
        Foo{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST_F(CompileDefclass, EvaluateNewInstanceFromDefaultSuperclass)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
            init() {}
        }
        Foo{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST_F(CompileDefclass, EvaluateNewInstanceFromSuperclass)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
            init() from Object() {}
        }
        Foo{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST_F(CompileDefclass, EvaluateNewInstanceWithDefaultInitializedMember)
{
    auto result = m_tester->runModule(R"(
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

TEST_F(CompileDefclass, EvaluateNewInstanceWithDefaultConstructorAndDefaultInitializedMember)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
            val i: Int = 100
        }
        Foo{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST_F(CompileDefclass, EvaluateDerefPublicVarMember)
{
    auto result = m_tester->runModule(R"(
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

TEST_F(CompileDefclass, EvaluateDerefPublicVarDefaultInitializedMember)
{
    auto result = m_tester->runModule(R"(
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

TEST_F(CompileDefclass, EvaluateDerefThisProtectedVarMember)
{
    auto result = m_tester->runModule(R"(
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

TEST_F(CompileDefclass, CompileDerefProtectedVarMemberFails)
{
    auto result = m_tester->compileModule(R"(
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

TEST_F(CompileDefclass, EvaluateDerefThisPrivateVarMember)
{
    auto result = m_tester->runModule(R"(
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

TEST_F(CompileDefclass, CompileDerefPrivateVarMemberFails)
{
    auto result = m_tester->compileModule(R"(
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

TEST_F(CompileDefclass, EvaluateInvokeMethod)
{
    auto result = m_tester->runModule(R"(
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

TEST_F(CompileDefclass, EvaluateDefGenericClass)
{
    auto result = m_tester->runModule(R"(
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

TEST_F(CompileDefclass, EvaluateInvokeGenericMethod)
{
    auto result = m_tester->runModule(R"(
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
        tuple.Element0 + tuple.Element1
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(DataCellInt(142))));
}

TEST_F(CompileDefclass, EvaluateInvokeGenericMethodForGenericClass)
{
    auto result = m_tester->runModule(R"(
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
        tuple.Element0 + tuple.Element1
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(DataCellInt(142))));
}

TEST_F(CompileDefclass, EvaluateNewInstanceOfSealedClass)
{
    auto result = m_tester->runModule(R"(
        defclass Foo sealed {
            val value: Int = 42
        }
        Foo{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST_F(CompileDefclass, CompileDefineSubclassOfFinalClassFails)
{
    auto result = m_tester->compileModule(R"(
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

TEST_F(CompileDefclass, EvaluateNewInstanceOfFinalClass)
{
    auto result = m_tester->runModule(R"(
        defclass Foo final {
            val value: Int = 42
        }
        Foo{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}
