
#include "base_symbolizer_fixture.h"

void
BaseSymbolizerFixture::SetUp()
{
    lyric_test::TesterOptions testerOptions;
    testerOptions.taskSettings = lyric_build::TaskSettings(tempo_config::ConfigMap{
        {"global", tempo_config::ConfigMap{
            {"bootstrapDirectoryPath", tempo_config::ConfigValue(LYRIC_BUILD_BOOTSTRAP_DIR)},
            {"sourceBaseUrl", tempo_config::ConfigValue("/src")},
        }},
    });

    m_tester = std::make_unique<lyric_test::LyricTester>(testerOptions);
    TU_RAISE_IF_NOT_OK (m_tester->configure());
}
