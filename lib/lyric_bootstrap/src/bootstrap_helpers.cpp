
#include <lyric_bootstrap/bootstrap_helpers.h>

lyric_common::ModuleLocation
lyric_bootstrap::preludeLocation()
{
    return lyric_common::ModuleLocation::fromString(BOOTSTRAP_PRELUDE_LOCATION);
}

lyric_common::SymbolUrl
lyric_bootstrap::preludeSymbol(std::string_view symbolName)
{
    return lyric_common::SymbolUrl(preludeLocation(), lyric_common::SymbolPath({std::string(symbolName)}));
}
