#ifndef LYRIC_BUILD_SYMBOL_MAP_H
#define LYRIC_BUILD_SYMBOL_MAP_H

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <lyric_common/assembly_location.h>
#include <lyric_common/symbol_url.h>

namespace lyric_build {

    typedef absl::flat_hash_map<
        lyric_common::AssemblyLocation,
        absl::flat_hash_set<lyric_common::SymbolPath>>
        LocationSymbolPathSetMap;

    class SymbolMap {

    public:
        SymbolMap();
        SymbolMap(const LocationSymbolPathSetMap &symbolMap);

        bool containsLocation(const lyric_common::AssemblyLocation &assemblyLocation) const;
        bool containsSymbol(const lyric_common::SymbolUrl &symbolUrl) const;

        LocationSymbolPathSetMap::const_iterator symbolsBegin() const;
        LocationSymbolPathSetMap::const_iterator symbolsEnd() const;

    private:
        LocationSymbolPathSetMap m_symbolMap;
    };
}

#endif // LYRIC_BUILD_SYMBOL_MAP_H
