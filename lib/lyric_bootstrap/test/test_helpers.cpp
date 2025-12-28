
#include <lyric_test/lyric_tester.h>

#include "test_helpers.h"

tempo_utils::Result<lyric_test::CompileModule>
compileModule(const std::string &code)
{
    lyric_test::TesterOptions testerOptions;
    testerOptions.bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>();
    return lyric_test::LyricTester::compileSingleModule(code, testerOptions);
}

tempo_utils::Result<lyric_test::RunModule>
runModule(const std::string &code)
{
    lyric_test::TesterOptions testerOptions;
    testerOptions.bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>();
    return lyric_test::LyricTester::runSingleModule(code, testerOptions);
}
