
#include "base_bootstrap_fixture.h"

void
BaseBootstrapFixture::SetUp()
{
    lyric_test::TesterOptions testerOptions;
    testerOptions.bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>();
    tester = std::make_unique<lyric_test::LyricTester>(testerOptions);
    TU_RAISE_IF_NOT_OK (tester->configure());
}

tempo_utils::Result<lyric_test::CompileModule>
BaseBootstrapFixture::compileModule(const std::string &code)
{
    return tester->compileModule(code);
}

tempo_utils::Result<lyric_test::RunModule>
BaseBootstrapFixture::runModule(const std::string &code)
{
    return tester->runModule(code);
}
