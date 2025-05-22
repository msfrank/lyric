#ifndef LYRIC_TEST_LYRIC_TESTER_H
#define LYRIC_TEST_LYRIC_TESTER_H

#include <filesystem>

#include <absl/container/flat_hash_map.h>

#include <lyric_build/task_settings.h>
#include <lyric_build/lyric_builder.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_test/test_runner.h>
#include <lyric_test/test_run.h>

namespace lyric_test {

    struct TesterOptions {
        std::filesystem::path testRootDirectory;
        bool useInMemoryCache = true;
        bool isTemporary = true;
        bool keepBuildOnUnexpectedResult = true;
        std::shared_ptr<lyric_build::TaskRegistry> taskRegistry;
        std::shared_ptr<lyric_runtime::AbstractLoader> fallbackLoader;
        absl::flat_hash_map<std::string,std::string> packageMap;
        lyric_build::TaskSettings overrides;
    };

    class LyricTester {

    public:
        explicit LyricTester(const TesterOptions &options = {});

        tempo_utils::Status configure();

        const TestRunner *getRunner() const;

        tempo_utils::Result<lyric_common::ModuleLocation> writeModule(
            const std::string &code,
            const std::filesystem::path &path = {});

        tempo_utils::Result<SymbolizeModule> symbolizeModule(
            const std::string &code,
            const std::filesystem::path &path = {});

        tempo_utils::Result<AnalyzeModule> analyzeModule(
            const std::string &code,
            const std::filesystem::path &path = {});

        tempo_utils::Result<CompileModule> compileModule(
            const std::string &code,
            const std::filesystem::path &path = {});

        tempo_utils::Result<PackageModule> packageModule(
            const lyric_packaging::PackageSpecifier &specifier,
            const std::string &code,
            const std::filesystem::path &path = {});

        tempo_utils::Result<RunModule> runModule(
            const std::string &code,
            const std::filesystem::path &path = {});

        tempo_utils::Result<RunModule> runModule(
            const lyric_packaging::PackageSpecifier &specifier,
            const std::string &code,
            const std::filesystem::path &path = {});

        static tempo_utils::Result<CompileModule> compileSingleModule(
            const std::string &code,
            const TesterOptions &options = {});

        static tempo_utils::Result<AnalyzeModule> analyzeSingleModule(
            const std::string &code,
            const TesterOptions &options = {});

        static tempo_utils::Result<SymbolizeModule> symbolizeSingleModule(
            const std::string &code,
            const TesterOptions &options = {});

        static tempo_utils::Result<PackageModule> packageSingleModule(
            const lyric_packaging::PackageSpecifier &specifier,
            const std::string &code,
            const TesterOptions &options = {});

        static tempo_utils::Result<RunModule> runSingleModule(
            const std::string &code,
            const TesterOptions &options = {});

        static tempo_utils::Result<RunModule> runSingleModule(
            const lyric_packaging::PackageSpecifier &specifier,
            const std::string &code,
            const TesterOptions &options = {});

    private:
        TesterOptions m_options;
        std::shared_ptr<TestRunner> m_runner;
    };
}

#endif // LYRIC_TEST_LYRIC_TESTER_H
