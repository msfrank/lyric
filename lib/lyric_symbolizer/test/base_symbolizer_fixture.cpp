
#include "base_symbolizer_fixture.h"

void
BaseSymbolizerFixture::SetUp()
{
    lyric_test::TesterOptions testerOptions;
    testerOptions.bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>(LYRIC_BUILD_BOOTSTRAP_DIR);
    m_tester = std::make_unique<lyric_test::LyricTester>(testerOptions);
    TU_RAISE_IF_NOT_OK (m_tester->configure());
}
