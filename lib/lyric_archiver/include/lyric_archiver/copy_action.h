#ifndef LYRIC_ARCHIVER_COPY_ACTION_H
#define LYRIC_ARCHIVER_COPY_ACTION_H

#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/object_root.h>
#include <lyric_common/symbol_url.h>
#include <lyric_importer/action_import.h>

#include "archiver_state.h"

namespace lyric_archiver {

    tempo_utils::Result<lyric_assembler::ActionSymbol *> copy_action(
        lyric_importer::ActionImport *actionImport,
        const std::string &importHash,
        lyric_assembler::NamespaceSymbol *targetNamespace,
        SymbolReferenceSet &symbolReferenceSet,
        ArchiverState &archiverState);

    tempo_utils::Status define_action(
        lyric_importer::ActionImport *actionImport,
        lyric_assembler::ActionSymbol *actionSymbol,
        const std::string &importHash,
        lyric_assembler::NamespaceSymbol *targetNamespace,
        SymbolReferenceSet &symbolReferenceSet,
        ArchiverState &archiverState);
}

#endif // LYRIC_ARCHIVER_COPY_ACTION_H
