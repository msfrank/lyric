#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <absl/strings/escaping.h>

#include <lyric_archiver/lyric_archiver.h>
#include <lyric_assembler/object_state.h>
#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>
#include <tempo_security/sha256_hash.h>

#include "archiver_tester.h"
#include "base_archiver_fixture.h"

TEST(ArchiveCall, SingleCall)
{
    lyric_test::TesterOptions testerOptions;
    testerOptions.buildConfig = tempo_config::ConfigMap{
        {"global", tempo_config::ConfigMap{
            {"bootstrapDirectoryPath", tempo_config::ConfigValue(LYRIC_BUILD_BOOTSTRAP_DIR)},
        }},
    };

    lyric_test::LyricTester tester(testerOptions);
    ASSERT_THAT (tester.configure(), tempo_test::IsOk());

    auto *runner = tester.getRunner();
    auto *builder = runner->getBuilder();
    auto sharedModuleCache = builder->getSharedModuleCache();

    auto mod1location = lyric_common::ModuleLocation::fromString("/mod1");

    auto compileMod1Result = tester.compileModule(R"(
        def call1(): Bool {
            true
        }
    )", "mod1");

    ASSERT_THAT (compileMod1Result, tempo_test::IsResult());
    auto compileMod1 = compileMod1Result.getResult();
    auto mod1object = compileMod1.getModule();

    std::shared_ptr<lyric_runtime::AbstractLoader> dependencyLoader;
    TU_ASSIGN_OR_RAISE (dependencyLoader, lyric_build::DependencyLoader::create(
        compileMod1.getComputation(), builder->getCache()));
    auto localModuleCache = lyric_importer::ModuleCache::create(dependencyLoader);

    lyric_archiver::ArchiverOptions options;
    options.preludeLocation = lyric_common::ModuleLocation::fromString(BOOTSTRAP_PRELUDE_LOCATION);

    auto location = lyric_common::ModuleLocation::fromString("/archive");
    auto recorder = tempo_tracing::TraceRecorder::create();
    lyric_archiver::LyricArchiver archiver(location, localModuleCache, sharedModuleCache, recorder, options);
    ASSERT_THAT (archiver.initialize(), tempo_test::IsOk());

    ASSERT_THAT (archiver.insertModule(mod1location, mod1object), tempo_test::IsOk());

    lyric_common::SymbolUrl call1symbol(mod1location, lyric_common::SymbolPath({"call1"}));
    ASSERT_THAT (archiver.archiveSymbol(call1symbol, "Call1", lyric_object::AccessType::Public), tempo_test::IsOk());

    lyric_object::LyricObject archive;
    TU_ASSIGN_OR_RAISE (archive, archiver.toObject());

    auto root = archive.getObject();
    ASSERT_EQ (3, root.numSymbols());

    auto hashBytes = tempo_security::Sha256Hash::hash(mod1location.toString());
    auto mod1Hash = absl::StrCat("$", absl::BytesToHexString(hashBytes));
    auto symbolWalker = root.findSymbol(lyric_common::SymbolPath({mod1Hash, "call1"}));
    ASSERT_TRUE (symbolWalker.isValid());

    ASSERT_THAT (localModuleCache->insertModule(location, archive), tempo_test::IsResult());

    auto runModuleResult = tester.runModule(R"(
        import from "/archive" { Call1 }
        Call1()
    )");

    ASSERT_THAT (runModuleResult, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

class ArchiveCallTests : public BaseArchiverFixture {
protected:
    ArchiveCallTests() = default;
};

TEST_F(ArchiveCallTests, RunSingleCall)
{
    ASSERT_THAT (configure(), tempo_test::IsOk());

    lyric_common::ModuleLocation mod1location;
    TU_ASSIGN_OR_RAISE (mod1location, writeModule(R"(
        def call1(): Bool {
            true
        }
    )", "mod1"));

    ASSERT_THAT (prepare(), tempo_test::IsOk());

    ASSERT_THAT (importModule(mod1location), tempo_test::IsOk());
    ASSERT_THAT (archiveSymbol(mod1location, "call1", "Call1"), tempo_test::IsOk());
    ASSERT_THAT (build(), tempo_test::IsOk());

    auto runModuleResult = runCode(R"(
        import from "dev.zuri.test:///archive" { call1 }
        Call1()
    )");

    ASSERT_THAT (runModuleResult, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}
