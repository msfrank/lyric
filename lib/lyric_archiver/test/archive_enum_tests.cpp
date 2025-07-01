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

class ArchiveEnumTests : public BaseArchiverFixture {
protected:
    ArchiveEnumTests() = default;
};

TEST_F(ArchiveEnumTests, ArchiveEnumAndCheckMember)
{
    ASSERT_THAT (configure("archive:///archive"), tempo_test::IsOk());

    lyric_common::ModuleLocation mod1location;
    TU_ASSIGN_OR_RAISE (mod1location, writeModule(R"(
        defenum FooEnum {
            case Foo
            case Bar
            case Baz
            case Qux
            val Field: Int = 42
        }
    )", "mod1"));

    ASSERT_THAT (prepare(), tempo_test::IsOk());

    ASSERT_THAT (importModule(mod1location), tempo_test::IsOk());

    lyric_common::SymbolUrl archivedUrl;
    TU_ASSIGN_OR_RAISE (archivedUrl, archiveSymbol(mod1location, "FooEnum"));
    lyric_assembler::BindingSymbol *bindingSymbol;
    TU_ASSIGN_OR_RAISE (bindingSymbol, declareBinding("FooEnumAlias", lyric_object::AccessType::Public));
    ASSERT_THAT (bindingSymbol->defineTarget(lyric_common::TypeDef::forConcrete(archivedUrl)), tempo_test::IsOk());

    ASSERT_THAT (build(), tempo_test::IsOk());

    auto runModuleResult = runCode(R"(
        import from "archive:///archive" { FooEnumAlias }
        FooEnumAlias.Field == 42
    )");

    ASSERT_THAT (runModuleResult, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(ArchiveEnumTests, ArchiveEnumAndCheckMethod)
{
    ASSERT_THAT (configure("archive:///archive"), tempo_test::IsOk());

    lyric_common::ModuleLocation mod1location;
    TU_ASSIGN_OR_RAISE (mod1location, writeModule(R"(
        defenum FooEnum {
            case Foo
            case Bar
            case Baz
            case Qux
            def FortyTwo(): Int {
                42
            }
        }
    )", "mod1"));

    ASSERT_THAT (prepare(), tempo_test::IsOk());

    ASSERT_THAT (importModule(mod1location), tempo_test::IsOk());

    lyric_common::SymbolUrl archivedUrl;
    TU_ASSIGN_OR_RAISE (archivedUrl, archiveSymbol(mod1location, "FooEnum"));
    lyric_assembler::BindingSymbol *bindingSymbol;
    TU_ASSIGN_OR_RAISE (bindingSymbol, declareBinding("FooEnumAlias", lyric_object::AccessType::Public));
    ASSERT_THAT (bindingSymbol->defineTarget(lyric_common::TypeDef::forConcrete(archivedUrl)), tempo_test::IsOk());

    ASSERT_THAT (build(), tempo_test::IsOk());

    auto runModuleResult = runCode(R"(
        import from "archive:///archive" { FooEnumAlias }
        FooEnumAlias.FortyTwo() == 42
    )");

    ASSERT_THAT (runModuleResult, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(ArchiveEnumTests, ArchiveEnumAndCheckImpl)
{
    ASSERT_THAT (configure("archive:///archive"), tempo_test::IsOk());

    lyric_common::ModuleLocation mod1location;
    TU_ASSIGN_OR_RAISE (mod1location, writeModule(R"(
        defenum FooEnum {
            case Foo
            case Bar
            case Baz
            case Qux
            impl Equality[FooEnum,FooEnum] {
                def Equals(lhs: FooEnum, rhs: FooEnum): Bool {
                    true
                }
            }
        }
    )", "mod1"));

    ASSERT_THAT (prepare(), tempo_test::IsOk());

    ASSERT_THAT (importModule(mod1location), tempo_test::IsOk());

    lyric_common::SymbolUrl archivedUrl;
    TU_ASSIGN_OR_RAISE (archivedUrl, archiveSymbol(mod1location, "FooEnum"));
    lyric_assembler::BindingSymbol *bindingSymbol;
    TU_ASSIGN_OR_RAISE (bindingSymbol, declareBinding("FooEnumAlias", lyric_object::AccessType::Public));
    ASSERT_THAT (bindingSymbol->defineTarget(lyric_common::TypeDef::forConcrete(archivedUrl)), tempo_test::IsOk());

    ASSERT_THAT (build(), tempo_test::IsOk());

    auto runModuleResult = runCode(R"(
        import from "archive:///archive" { FooEnumAlias }
        using FooEnumAlias
        FooEnumAlias == FooEnumAlias
    )");

    ASSERT_THAT (runModuleResult, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}
