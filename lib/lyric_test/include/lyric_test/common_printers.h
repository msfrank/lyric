#ifndef LYRIC_TEST_COMMON_PRINTERS_H
#define LYRIC_TEST_COMMON_PRINTERS_H

#include <lyric_common/module_location.h>
#include <lyric_common/symbol_path.h>
#include <lyric_common/symbol_url.h>

namespace lyric_common {

    void PrintTo(const ModuleLocation &moduleLocation, std::ostream* os);

    void PrintTo(const SymbolUrl &symbolUrl, std::ostream* os);

    void PrintTo(const SymbolPath &symbolPath, std::ostream* os);
}

#endif // LYRIC_TEST_COMMON_PRINTERS_H
