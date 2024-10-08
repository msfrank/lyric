#ifndef LYRIC_BOOTSTRAP_BOOTSTRAP_HELPERS_H
#define LYRIC_BOOTSTRAP_BOOTSTRAP_HELPERS_H

#include <lyric_common/module_location.h>
#include <lyric_common/symbol_url.h>

namespace lyric_bootstrap {

    lyric_common::ModuleLocation preludeLocation();

    lyric_common::SymbolUrl preludeSymbol(const lyric_common::SymbolPath &symbolPath);

    lyric_common::SymbolUrl preludeSymbol(std::string_view symbolPathString);

}

#endif // LYRIC_BOOTSTRAP_BOOTSTRAP_HELPERS_H
