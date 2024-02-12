#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <string>

#include <lyric_common/assembly_location.h>
#include <lyric_common/symbol_url.h>

lyric_common::AssemblyLocation preludeLocation();

lyric_common::SymbolUrl preludeSymbol(std::string_view symbolName);

#endif // TEST_HELPERS_H
