#include <gtest/gtest.h>

#include <tempo_command/command.h>
#include <tempo_command/logging_environment.h>
#include <tempo_config/base_conversions.h>
#include <tempo_utils/logging.h>

class TestEnvironment : public ::testing::Environment {
public:
    explicit TestEnvironment(std::filesystem::path loggingConfigFile)
        : m_loggingConfigFile(std::move(loggingConfigFile)) {}

    void SetUp() override {
        tempo_command::init_logging_from_environment(m_loggingConfigFile);
    }
    void TearDown() override {
        tempo_utils::cleanup_logging();
    }

private:
    std::filesystem::path m_loggingConfigFile;
};

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    tempo_command::Command command("tempo_testsuite");
    command.addOption("loggingConfigFile", {"--logging-config-file"},
        tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
        "load logging config from FILE", "FILE");

    command.parse(argc, (const char **) argv);

    tempo_config::PathParser loggingConfigFileParser(std::filesystem::path{});
    std::filesystem::path loggingConfigFile;
    TU_RAISE_IF_NOT_OK (command.convert(loggingConfigFile, loggingConfigFileParser, "loggingConfigFile"));

    ::testing::AddGlobalTestEnvironment(new TestEnvironment(loggingConfigFile));

    return RUN_ALL_TESTS();
}
