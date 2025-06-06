#ifndef LYRIC_TEST_LYRIC_PROTOCOL_TESTER_H
#define LYRIC_TEST_LYRIC_PROTOCOL_TESTER_H

#include <filesystem>

#include <absl/container/flat_hash_map.h>

#include <lyric_build/task_settings.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_test/base_protocol_mock.h>
#include <lyric_test/test_runner.h>

namespace lyric_test {

    struct ProtocolTesterOptions {
        std::filesystem::path testRootDirectory = {};
        bool useInMemoryCache = true;
        bool isTemporary = true;
        bool keepBuildOnUnexpectedResult = true;
        std::shared_ptr<lyric_build::TaskRegistry> taskRegistry = {};
        std::shared_ptr<lyric_runtime::AbstractLoader> bootstrapLoader = {};
        std::shared_ptr<lyric_runtime::AbstractLoader> fallbackLoader = {};
        lyric_build::TaskSettings taskSettings = {};
        absl::flat_hash_map<
            tempo_utils::Url,
            std::shared_ptr<BaseProtocolMock>> protocolMocks = {};
    };

    class LyricProtocolTester {

    public:
        explicit LyricProtocolTester(const ProtocolTesterOptions &options);

        tempo_utils::Status configure();

        const TestRunner *getRunner() const;

        tempo_utils::Result<RunModule> runModuleInMockSandbox(
            const std::string &code,
            const std::filesystem::path &modulePath = {},
            const std::filesystem::path &baseDir = {});

        static tempo_utils::Result<RunModule> runSingleModuleInMockSandbox(
            const std::string &code,
            const ProtocolTesterOptions &options = {});

    private:
        ProtocolTesterOptions m_options;
        std::shared_ptr<TestRunner> m_runner;
    };
}

#endif // LYRIC_TEST_LYRIC_PROTOCOL_TESTER_H