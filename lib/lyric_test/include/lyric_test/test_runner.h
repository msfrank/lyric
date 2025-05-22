#ifndef LYRIC_TEST_TEST_RUNNER_H
#define LYRIC_TEST_TEST_RUNNER_H

#include <filesystem>

#include <absl/container/flat_hash_map.h>

#include <lyric_build/config_store.h>
#include <lyric_build/lyric_builder.h>
#include <lyric_packaging/package_specifier.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/gc_heap.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_test/abstract_tester.h>
#include <lyric_test/test_result.h>
#include <lyric_test/test_run.h>

namespace lyric_test {

    class TestRunner : public AbstractTester, public std::enable_shared_from_this<TestRunner> {

    public:
        static std::shared_ptr<TestRunner> create(
            const std::filesystem::path &testRootDirectory,
            bool useInMemoryCache,
            bool isTemporary,
            bool keepBuildOnUnexpectedResult,
            const std::string &preludeLocation,
            std::shared_ptr<lyric_build::TaskRegistry> taskRegistry,
            std::shared_ptr<lyric_runtime::AbstractLoader> fallbackLoader,
            const absl::flat_hash_map<std::string,std::string> &packageMap,
            const tempo_config::ConfigMap &buildConfig,
            const tempo_config::ConfigMap &buildVendorConfig);
        ~TestRunner() override;

        std::filesystem::path getTesterDirectory() const override;
        std::filesystem::path getInstallDirectory() const override;
        lyric_build::LyricBuilder *getBuilder() const override;
        tempo_tracing::TempoSpanset getDiagnostics(
            const lyric_build::TargetComputation &computation) const override;

        tempo_utils::Status configureBaseTester();

        bool isConfigured() const;

        tempo_utils::Result<std::filesystem::path> writeNamedFileInternal(
            const std::filesystem::path &baseDir,
            const std::filesystem::path &path,
            const std::string &code);
        tempo_utils::Result<std::filesystem::path> writeTempFileInternal(
            const std::filesystem::path &baseDir,
            const std::filesystem::path &templatePath,
            const std::string &code);
        tempo_utils::Result<lyric_common::ModuleLocation> writeModuleInternal(
            const std::string &code,
            const std::filesystem::path &path = {});

        tempo_utils::Result<lyric_build::TargetComputationSet>
        computeTargetInternal(
            const lyric_build::TaskId &target,
            const lyric_build::ConfigStore &overrides = {});

        tempo_utils::Result<BuildModule> buildModuleInternal(
            const std::string &code,
            const std::filesystem::path &path = {});
        tempo_utils::Result<CompileModule> compileModuleInternal(
            const std::string &code,
            const std::filesystem::path &path = {});
        tempo_utils::Result<AnalyzeModule> analyzeModuleInternal(
            const std::string &code,
            const std::filesystem::path &path = {});
        tempo_utils::Result<SymbolizeModule> symbolizeModuleInternal(
            const std::string &code,
            const std::filesystem::path &path = {});
        tempo_utils::Result<PackageModule> packageModuleInternal(
            const lyric_packaging::PackageSpecifier &specifier,
            const std::string &code,
            const std::filesystem::path &path = {});

        tempo_utils::Result<PackageModule> packageTargetsInternal(
            const absl::flat_hash_set<lyric_build::TaskId> &targets,
            const lyric_common::ModuleLocation &mainLocation,
            const lyric_packaging::PackageSpecifier &specifier);
        tempo_utils::Result<PackageModule> packageWorkspaceInternal(
            const lyric_common::ModuleLocation &mainLocation,
            const lyric_packaging::PackageSpecifier &specifier);

        bool containsUnexpectedResult() const;
        void setUnexpectedResult(bool unexpectedResult);

    private:
        std::filesystem::path m_testRootDirectory;
        bool m_useInMemoryCache;
        bool m_isTemporary;
        bool m_keepBuildOnUnexpectedResult;
        std::string m_preludeLocation;
        std::shared_ptr<lyric_build::TaskRegistry> m_taskRegistry;
        std::shared_ptr<lyric_runtime::AbstractLoader> m_fallbackLoader;
        absl::flat_hash_map<std::string,std::string> m_packageMap;
        const tempo_config::ConfigMap m_buildConfig;
        const tempo_config::ConfigMap m_buildVendorConfig;

        bool m_configured;
        std::filesystem::path m_testerDirectory;
        std::filesystem::path m_installDirectory;
        lyric_build::ConfigStore m_config;
        lyric_build::LyricBuilder *m_builder;
        bool m_unexpectedResult;

        TestRunner(
            const std::filesystem::path &testRootDirectory,
            bool useInMemoryCache,
            bool isTemporary,
            bool keepBuildOnUnexpectedResult,
            const std::string &preludeLocation,
            std::shared_ptr<lyric_build::TaskRegistry> taskRegistry,
            std::shared_ptr<lyric_runtime::AbstractLoader> fallbackLoader,
            const absl::flat_hash_map<std::string,std::string> &packageMap,
            const tempo_config::ConfigMap &buildConfig,
            const tempo_config::ConfigMap &buildVendorConfig);
    };
}

#endif // LYRIC_TEST_TEST_RUNNER_H