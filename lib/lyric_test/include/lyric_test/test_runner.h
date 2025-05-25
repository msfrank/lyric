#ifndef LYRIC_TEST_TEST_RUNNER_H
#define LYRIC_TEST_TEST_RUNNER_H

#include <filesystem>

#include <lyric_build/task_settings.h>
#include <lyric_build/lyric_builder.h>
#include <lyric_runtime/bytecode_interpreter.h>
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
            std::shared_ptr<lyric_build::TaskRegistry> taskRegistry,
            std::shared_ptr<lyric_runtime::AbstractLoader> bootstrapLoader,
            std::shared_ptr<lyric_runtime::AbstractLoader> fallbackLoader,
            const lyric_build::TaskSettings &taskSettings);
        ~TestRunner() override;

        std::filesystem::path getTesterDirectory() const override;
        lyric_build::LyricBuilder *getBuilder() const override;
        tempo_tracing::TempoSpanset getDiagnostics(
            const lyric_build::TargetComputation &computation) const override;

        tempo_utils::Status configureBaseTester();

        bool isConfigured() const;

        tempo_utils::Result<std::filesystem::path> writeNamedFileInternal(
            const std::string &code,
            const std::filesystem::path &filePath,
            const std::filesystem::path &baseDir = {});
        tempo_utils::Result<std::filesystem::path> writeTempFileInternal(
            const std::string &code,
            const std::filesystem::path &templatePath,
            const std::filesystem::path &baseDir = {});
        tempo_utils::Result<lyric_common::ModuleLocation> writeModuleInternal(
            const std::string &code,
            const std::filesystem::path &modulePath = {},
            const std::filesystem::path &baseDir = {});

        tempo_utils::Result<lyric_build::TargetComputationSet>
        computeTargetInternal(
            const lyric_build::TaskId &target,
            const lyric_build::TaskSettings &overrides = {});

        tempo_utils::Result<BuildModule> buildModuleInternal(
            const std::string &code,
            const std::filesystem::path &modulePath = {},
            const std::filesystem::path &baseDir = {});
        tempo_utils::Result<CompileModule> compileModuleInternal(
            const std::string &code,
            const std::filesystem::path &modulePath = {},
            const std::filesystem::path &baseDir = {});
        tempo_utils::Result<AnalyzeModule> analyzeModuleInternal(
            const std::string &code,
            const std::filesystem::path &modulePath = {},
            const std::filesystem::path &baseDir = {});
        tempo_utils::Result<SymbolizeModule> symbolizeModuleInternal(
            const std::string &code,
            const std::filesystem::path &modulePath = {},
            const std::filesystem::path &baseDir = {});

        bool containsUnexpectedResult() const;
        void setUnexpectedResult(bool unexpectedResult);

    private:
        std::filesystem::path m_testRootDirectory;
        bool m_useInMemoryCache;
        bool m_isTemporary;
        bool m_keepBuildOnUnexpectedResult;
        std::shared_ptr<lyric_build::TaskRegistry> m_taskRegistry;
        std::shared_ptr<lyric_runtime::AbstractLoader> m_bootstrapLoader;
        std::shared_ptr<lyric_runtime::AbstractLoader> m_fallbackLoader;
        lyric_build::TaskSettings m_taskSettings;

        bool m_configured;
        std::filesystem::path m_testerDirectory;
        lyric_build::LyricBuilder *m_builder;
        bool m_unexpectedResult;

        TestRunner(
            const std::filesystem::path &testRootDirectory,
            bool useInMemoryCache,
            bool isTemporary,
            bool keepBuildOnUnexpectedResult,
            std::shared_ptr<lyric_build::TaskRegistry> taskRegistry,
            std::shared_ptr<lyric_runtime::AbstractLoader> bootstrapLoader,
            std::shared_ptr<lyric_runtime::AbstractLoader> fallbackLoader,
            const lyric_build::TaskSettings &overrides);
    };
}

#endif // LYRIC_TEST_TEST_RUNNER_H