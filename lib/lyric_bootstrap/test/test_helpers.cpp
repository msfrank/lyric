
#include <lyric_bootstrap/bootstrap_types.h>
#include <lyric_test/lyric_tester.h>

#include "test_helpers.h"

lyric_common::AssemblyLocation
preludeLocation()
{
    return lyric_common::AssemblyLocation::fromString(lyric_bootstrap::kLyricBootstrapPrelude);
}

lyric_common::SymbolUrl
preludeSymbol(std::string_view symbolName)
{
    return lyric_common::SymbolUrl(preludeLocation(), lyric_common::SymbolPath({std::string(symbolName)}));
}

tempo_utils::Result<lyric_test::CompileModule>
compileModule(const std::string &code)
{
    lyric_test::TesterOptions testerOptions;
    testerOptions.buildConfig = tempo_config::ConfigMap{
        {"global", tempo_config::ConfigMap{
            {"bootstrapDirectoryPath", tempo_config::ConfigValue(LYRIC_BUILD_BOOTSTRAP_DIR)},
        }},
    };

    return lyric_test::LyricTester::compileSingleModule(code, testerOptions);
}

tempo_utils::Result<lyric_test::RunModule>
runModule(const std::string &code)
{
    lyric_test::TesterOptions testerOptions;
    testerOptions.buildConfig = tempo_config::ConfigMap{
        {"global", tempo_config::ConfigMap{
            {"bootstrapDirectoryPath", tempo_config::ConfigValue(LYRIC_BUILD_BOOTSTRAP_DIR)},
        }},
    };

    return lyric_test::LyricTester::runSingleModule(code, testerOptions);
}
