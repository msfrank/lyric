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
                     OperandRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST_F(CompileDefclass, EvaluateNewInstanceWithDefaultSuperInit)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
            init() {}
        }
        Foo{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     OperandRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST_F(CompileDefclass, EvaluateNewInstanceWithExplicitSuperInit)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
            init() {}
        }
        defclass Bar {
            init() from super() {}
        }
        Bar{}
    )");

    ASSERT_THAT (result,
        tempo_test::ContainsResult(RunModule(
            OperandRef(lyric_common::SymbolPath({"Bar"})))));
}

TEST_F(CompileDefclass, EvaluateNewInstanceWithExplicitNamedSuperInit)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
            init Named() {}
        }
        defclass Bar from Foo {
            init() from super.Named() {}
        }
        Bar{}
    )");

    ASSERT_THAT (result,
        tempo_test::ContainsResult(RunModule(
            OperandRef(lyric_common::SymbolPath({"Bar"})))));
}

TEST_F(CompileDefclass, EvaluateNewInstanceWithExplicitThisInit)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
            val Value: Int
            init(i: Int) {
                this.Value = i
            }
            init Named() from this(42) {}
        }
        Foo.Named{}.Value
    )");

    ASSERT_THAT (result,
        tempo_test::ContainsResult(RunModule(
            OperandInt(42))));
}

TEST_F(CompileDefclass, EvaluateNewInstanceWithExplicitNamedThisInit)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
            val Value: Int
            init Named(i: Int) {
                this.Value = i
            }
            init() from this.Named(42) {}
        }
        Foo{}.Value
    )");

    ASSERT_THAT (result,
        tempo_test::ContainsResult(RunModule(
            OperandInt(42))));
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
                     OperandRef(lyric_common::SymbolPath({"Foo"})))));
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
                     OperandRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST_F(CompileDefclass, EvaluateDerefPublicVarMember)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
            var Index: Int
            init(i: Int) {
                this.Index = i
            }
        }
        var foo: Foo = Foo{100}
        foo.Index
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(OperandInt(100))));
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
                     OperandInt(100))));
}

TEST_F(CompileDefclass, EvaluateDerefThisPrivateVarMember)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
            var _index: Int
            init(i: Int) {
                this._index = i
            }
            def index(): Int {
                this._index
            }
        }
        defclass Bar from Foo {
            init(i: Int) from super(i) {}
            def Add(i: Int): Int {
                i + this.index()
            }
        }
        var bar: Bar = Bar{100}
        bar.Add(100)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandInt(200))));
}

TEST_F(CompileDefclass, CompileDerefPrivateVarMemberFails)
{
    auto result = m_tester->compileModule(R"(
        defclass Foo {
            var _index: Int
            init(i: Int) {
                this._index = i
            }
        }
        var foo: Foo = Foo{100}
        foo._index
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_assembler::AssemblerCondition::kInvalidAccess))));
}

TEST_F(CompileDefclass, EvaluateInvokePublicMethod)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
            var _index: Int
            init(i: Int) {
                this._index = i
            }
            def Index(): Int {
                this._index
            }
        }
        var foo: Foo = Foo{100}
        foo.Index()
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(OperandInt(100))));
}

TEST_F(CompileDefclass, EvaluateInvokePrivateMethodFromInheritedClass)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
            def _Index(): Int {
                1
            }
        }
        defclass Bar from Foo {
            init() {}
            def Index(): Int {
                this._Index()
            }
        }
        val bar = Bar{}
        bar.Index()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandInt(1))));
}

TEST_F(CompileDefclass, EvaluateInvokeMethodWithNoReturnType)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
            var _index: Int
            init(i: Int) {
                this._index = i
            }
            def NoReturn() {
                this._index + 1
            }
        }
        var foo: Foo = Foo{100}
        foo.NoReturn()
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     MatchesDataCellType(lyric_runtime::OperandType::Invalid))));
}

TEST_F(CompileDefclass, EvaluateInvokeVirtualMethodOverridingBaseMethod)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
            def Index(): Int {
                1
            }
        }
        defclass Bar from Foo {
            init() {}
            def Index(): Int {
                2
            }
        }
        val foo: Foo = Bar{}
        foo.Index()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandInt(2))));
}

TEST_F(CompileDefclass, EvaluateInvokeVirtualMethodOverridingBaseStub)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
            decl Index(): Int
        }
        defclass Bar from Foo {
            init() {}
            def Index(): Int {
                2
            }
        }
        val foo: Foo = Bar{}
        foo.Index()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandInt(2))));
}

TEST_F(CompileDefclass, CompileDefineStubOverridingMethodFails)
{
    auto result = m_tester->compileModule(R"(
        defclass Foo {
            def Index(): Int {
                1
            }
        }
        defclass Bar from Foo {
            decl Index(): Int
        }
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_assembler::AssemblerCondition::kSymbolAlreadyDefined))));
}

TEST_F(CompileDefclass, CompileDefineStubOverridingStubFails)
{
    auto result = m_tester->compileModule(R"(
        defclass Foo {
            decl Index(): Int
        }
        defclass Bar from Foo {
            decl Index(): Int
        }
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_assembler::AssemblerCondition::kSymbolAlreadyDefined))));
}

TEST_F(CompileDefclass, CompileDefineMethodOverridingFinalMethodFails)
{
    auto result = m_tester->compileModule(R"(
        defclass Foo {
            def Index(): Int final {
                1
            }
        }
        defclass Bar from Foo {
            def Index(): Int {
                2
            }
        }
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_assembler::AssemblerCondition::kSymbolAlreadyDefined))));
}

TEST_F(CompileDefclass, EvaluateDefGenericClass)
{
    auto result = m_tester->runModule(R"(
        defclass Foo[A] {
            var _index: A
            init(i: A) {
                this._index = i
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
                     OperandInt(100))));
}

TEST_F(CompileDefclass, EvaluateInvokeGenericMethod)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
            var _index: Int
            init(i: Int) {
                this._index = i
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
                 tempo_test::ContainsResult(RunModule(OperandInt(142))));
}

TEST_F(CompileDefclass, EvaluateInvokeGenericMethodForGenericClass)
{
    auto result = m_tester->runModule(R"(
        defclass Foo[S] {
            var _s: S
            init(s: S) {
                this._s = s
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
                 tempo_test::ContainsResult(RunModule(OperandInt(142))));
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
                     OperandRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST_F(CompileDefclass, CompileDefineSubclassOfFinalClassFails)
{
    auto result = m_tester->compileModule(R"(
        defclass Foo final {
        }
        defclass Bar from Foo {
            init() {}
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
                     OperandRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST_F(CompileDefclass, EvaluateDerefGlobalMember)
{
    auto result = m_tester->runModule(R"(
        defclass Foo final {
            global {
                val GlobalValue: Int = 42
            }
        }
        Foo.GlobalValue
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandInt(42))));
}

TEST_F(CompileDefclass, EvaluateInvokeGlobalMethod)
{
    auto result = m_tester->runModule(R"(
        defclass Foo final {
            global {
                def GetValue(): Int { 42 }
            }
        }
        Foo.GetValue()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandInt(42))));
}

TEST_F(CompileDefclass, EvaluateNewInstanceWithImpl)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
            impl Equality[Foo,Foo] {
                def Equals(lhs: Foo, rhs: Foo): Bool { true }
            }
        }
        Foo{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     OperandRef(lyric_common::SymbolPath({"Foo"})))));
}
