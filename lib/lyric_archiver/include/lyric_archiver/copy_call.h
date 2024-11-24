#ifndef LYRIC_ARCHIVER_COPY_CALL_H
#define LYRIC_ARCHIVER_COPY_CALL_H

#include <lyric_assembler/object_root.h>
#include <lyric_common/symbol_url.h>
#include <lyric_importer/call_import.h>

#include "archiver_state.h"

namespace lyric_archiver {

    tempo_utils::Result<lyric_common::SymbolUrl> copy_call(
        lyric_importer::CallImport *callImport,
        const std::string &importHash,
        lyric_assembler::NamespaceSymbol *targetNamespace,
        ArchiverState &archiverState);
}

#endif // LYRIC_ARCHIVER_COPY_CALL_H
