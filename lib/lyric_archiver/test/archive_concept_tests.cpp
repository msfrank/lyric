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

class ArchiveConceptTests : public BaseArchiverFixture {
protected:
    ArchiveConceptTests() = default;
};

TEST_F(ArchiveConceptTests, ArchiveConceptAndCheckMethod)
{
    ASSERT_THAT (configure("archive:///archive"), tempo_test::IsOk());

    lyric_common::ModuleLocation mod1location;
    TU_ASSIGN_OR_RAISE (mod1location, writeModule(R"(
        defconcept FooConcept {
            decl FortyTwo(): Int
        }
    )", "mod1"));

    ASSERT_THAT (prepare(), tempo_test::IsOk());

    ASSERT_THAT (importModule(mod1location), tempo_test::IsOk());

    lyric_common::SymbolUrl archivedUrl;
    TU_ASSIGN_OR_RAISE (archivedUrl, archiveSymbol(mod1location, "FooConcept"));
    lyric_assembler::BindingSymbol *bindingSymbol;
    TU_ASSIGN_OR_RAISE (bindingSymbol, declareBinding("FooConceptAlias", lyric_object::AccessType::Public));
    ASSERT_THAT (bindingSymbol->defineTarget(lyric_common::TypeDef::forConcrete(archivedUrl)), tempo_test::IsOk());

    ASSERT_THAT (build(), tempo_test::IsOk());

    auto runModuleResult = runCode(R"(
        import from "archive:///archive" { FooConceptAlias }
        defclass FooClass {
            impl FooConceptAlias {
                def FortyTwo(): Int { 42 }
            }
        }
        val foo: FooConceptAlias = FooClass{}
        foo.FortyTwo() == 42
    )");

    ASSERT_THAT (runModuleResult, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}
