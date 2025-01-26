#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <absl/strings/escaping.h>

#include <lyric_archiver/lyric_archiver.h>
#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/object_state.h>
#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>
#include <tempo_security/sha256_hash.h>

#include "base_archiver_fixture.h"

class ArchiveStructTests : public BaseArchiverFixture {
protected:
    ArchiveStructTests() = default;
};

TEST_F(ArchiveStructTests, ArchiveStructAndCheckMember)
{
    ASSERT_THAT (configure(), tempo_test::IsOk());

    lyric_common::ModuleLocation mod1location;
    TU_ASSIGN_OR_RAISE (mod1location, writeModule(R"(
        defstruct FooStruct {
            val Field: Int = 42
        }
    )", "mod1"));

    ASSERT_THAT (prepare(), tempo_test::IsOk());

    ASSERT_THAT (importModule(mod1location), tempo_test::IsOk());

    lyric_common::SymbolUrl archivedUrl;
    TU_ASSIGN_OR_RAISE (archivedUrl, archiveSymbol(mod1location, "FooStruct"));
    lyric_assembler::BindingSymbol *bindingSymbol;
    TU_ASSIGN_OR_RAISE (bindingSymbol, declareBinding("FooStructAlias", lyric_object::AccessType::Public));
    ASSERT_THAT (bindingSymbol->defineTarget(lyric_common::TypeDef::forConcrete(archivedUrl)), tempo_test::IsOk());

    ASSERT_THAT (build(), tempo_test::IsOk());

    auto runModuleResult = runCode(R"(
        import from "dev.zuri.test:///archive" { FooStructAlias }
        val foo: FooStructAlias = FooStructAlias{}
        foo.Field == 42
    )");

    ASSERT_THAT (runModuleResult, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(ArchiveStructTests, ArchiveStructAndCheckMethod)
{
    ASSERT_THAT (configure(), tempo_test::IsOk());

    lyric_common::ModuleLocation mod1location;
    TU_ASSIGN_OR_RAISE (mod1location, writeModule(R"(
        defstruct FooStruct {
            def FortyTwo(): Int {
                42
            }
        }
    )", "mod1"));

    ASSERT_THAT (prepare(), tempo_test::IsOk());

    ASSERT_THAT (importModule(mod1location), tempo_test::IsOk());

    lyric_common::SymbolUrl archivedUrl;
    TU_ASSIGN_OR_RAISE (archivedUrl, archiveSymbol(mod1location, "FooStruct"));
    lyric_assembler::BindingSymbol *bindingSymbol;
    TU_ASSIGN_OR_RAISE (bindingSymbol, declareBinding("FooStructAlias", lyric_object::AccessType::Public));
    ASSERT_THAT (bindingSymbol->defineTarget(lyric_common::TypeDef::forConcrete(archivedUrl)), tempo_test::IsOk());

    ASSERT_THAT (build(), tempo_test::IsOk());

    auto runModuleResult = runCode(R"(
        import from "dev.zuri.test:///archive" { FooStructAlias }
        val foo: FooStructAlias = FooStructAlias{}
        foo.FortyTwo() == 42
    )");

    ASSERT_THAT (runModuleResult, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(ArchiveStructTests, ArchiveStructAndCheckImpl)
{
    ASSERT_THAT (configure(), tempo_test::IsOk());

    lyric_common::ModuleLocation mod1location;
    TU_ASSIGN_OR_RAISE (mod1location, writeModule(R"(
        defstruct FooStruct {
            impl Equality[FooStruct,FooStruct] {
                def equals(lhs: FooStruct, rhs: FooStruct): Bool {
                    true
                }
            }
        }
    )", "mod1"));

    ASSERT_THAT (prepare(), tempo_test::IsOk());

    ASSERT_THAT (importModule(mod1location), tempo_test::IsOk());

    lyric_common::SymbolUrl archivedUrl;
    TU_ASSIGN_OR_RAISE (archivedUrl, archiveSymbol(mod1location, "FooStruct"));
    lyric_assembler::BindingSymbol *bindingSymbol;
    TU_ASSIGN_OR_RAISE (bindingSymbol, declareBinding("FooStructAlias", lyric_object::AccessType::Public));
    ASSERT_THAT (bindingSymbol->defineTarget(lyric_common::TypeDef::forConcrete(archivedUrl)), tempo_test::IsOk());

    ASSERT_THAT (build(), tempo_test::IsOk());

    auto runModuleResult = runCode(R"(
        import from "dev.zuri.test:///archive" { FooStructAlias }
        val foo1: FooStructAlias = FooStructAlias{}
        val foo2: FooStructAlias = FooStructAlias{}
        using foo1
        foo1 == foo2
    )");

    ASSERT_THAT (runModuleResult, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}
