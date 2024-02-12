
#include <lyric_bootstrap/bootstrap_types.h>

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
