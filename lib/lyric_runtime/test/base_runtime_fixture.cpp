
#include "base_runtime_fixture.h"

void
BaseRuntimeFixture::SetUp()
{
    lyric_test::TesterOptions testerOptions;
    testerOptions.bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>();
    staticLoader = std::make_shared<lyric_runtime::StaticLoader>();
    testerOptions.fallbackLoader = staticLoader;
    tester = std::make_unique<lyric_test::LyricTester>(testerOptions);
    TU_RAISE_IF_NOT_OK (tester->configure());
}
