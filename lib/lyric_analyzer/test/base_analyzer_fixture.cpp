
#include "base_analyzer_fixture.h"

void
BaseAnalyzerFixture::SetUp()
{
    tempo_utils::LoggingConfiguration loggingConf;
    loggingConf.severityFilter = tempo_utils::SeverityFilter::kVeryVerbose;
    tempo_utils::init_logging(loggingConf);
    lyric_test::TesterOptions testerOptions;
    testerOptions.bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>();
    m_tester = std::make_unique<lyric_test::LyricTester>(testerOptions);
    TU_RAISE_IF_NOT_OK (m_tester->configure());
}
