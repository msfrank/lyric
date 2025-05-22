
#include "base_analyzer_fixture.h"

void
BaseAnalyzerFixture::SetUp()
{
    lyric_test::TesterOptions testerOptions;
    testerOptions.overrides = lyric_build::TaskSettings(tempo_config::ConfigMap{
            {"global", tempo_config::ConfigMap{
                {"preludeLocation", tempo_config::ConfigValue(BOOTSTRAP_PRELUDE_LOCATION)},
                {"bootstrapDirectoryPath", tempo_config::ConfigValue(LYRIC_BUILD_BOOTSTRAP_DIR)},
                {"sourceBaseUrl", tempo_config::ConfigValue("/src")},
            }},
        });

    m_tester = std::make_unique<lyric_test::LyricTester>(testerOptions);
    TU_RAISE_IF_NOT_OK (m_tester->configure());
}
