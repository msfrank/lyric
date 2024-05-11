#ifndef LYRIC_ASSEMBLER_ABSTRACT_IMPORTER_H
#define LYRIC_ASSEMBLER_ABSTRACT_IMPORTER_H

#include <lyric_common/symbol_url.h>
#include <tempo_utils/result.h>

#include "assembly_state.h"

namespace lyric_assembler {

    class AbstractImporter {

    public:
        virtual ~AbstractImporter() = default;

        virtual tempo_utils::Result<lyric_common::SymbolUrl> importSymbol(
            const lyric_common::AssemblyLocation &assemblyLocation,
            const lyric_common::SymbolPath &symbolPath,
            AssemblyState *state) = 0;

        virtual tempo_utils::Result<lyric_common::SymbolUrl> importSymbol(
            const lyric_common::SymbolUrl &symbolUrl,
            AssemblyState *state) = 0;

    };
}

#endif // LYRIC_ASSEMBLER_ABSTRACT_IMPORTER_H
