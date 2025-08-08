#include <gtest/gtest.h>

#include <tempo_utils/logging.h>
#include <tempo_utils/log_sink.h>

class TestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        tempo_utils::LoggingConfiguration logConfiguration;
        logConfiguration.severityFilter = tempo_utils::SeverityFilter::kVeryVerbose;
        tempo_utils::init_logging(logConfiguration);
    }
    void TearDown() override {
        tempo_utils::cleanup_logging();
    }
};

int main(int argc, char **argv)
{
    ::testing::AddGlobalTestEnvironment(new TestEnvironment());
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
