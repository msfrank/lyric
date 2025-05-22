
#include <lyric_test/lyric_tester.h>

#include "test_helpers.h"

tempo_utils::Result<lyric_test::CompileModule>
compileModule(const std::string &code)
{
    lyric_test::TesterOptions testerOptions;
    testerOptions.overrides = lyric_build::TaskSettings(tempo_config::ConfigMap{
        {"global", tempo_config::ConfigMap{
            {"bootstrapDirectoryPath", tempo_config::ConfigValue(LYRIC_BUILD_BOOTSTRAP_DIR)},
        }},
    });

    return lyric_test::LyricTester::compileSingleModule(code, testerOptions);
}

tempo_utils::Result<lyric_test::RunModule>
runModule(const std::string &code)
{
    lyric_test::TesterOptions testerOptions;
    testerOptions.overrides = lyric_build::TaskSettings(tempo_config::ConfigMap{
        {"global", tempo_config::ConfigMap{
            {"bootstrapDirectoryPath", tempo_config::ConfigValue(LYRIC_BUILD_BOOTSTRAP_DIR)},
        }},
    });

    return lyric_test::LyricTester::runSingleModule(code, testerOptions);
}
