#ifndef LYRIC_ARCHIVER_COPY_CALL_H
#define LYRIC_ARCHIVER_COPY_CALL_H

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/object_root.h>
#include <lyric_common/symbol_url.h>
#include <lyric_importer/call_import.h>

#include "archiver_state.h"

namespace lyric_archiver {

    tempo_utils::Result<lyric_assembler::CallSymbol *> copy_call(
        lyric_importer::CallImport *callImport,
        const std::string &importHash,
        lyric_assembler::NamespaceSymbol *targetNamespace,
        SymbolReferenceSet &symbolReferenceSet,
        ArchiverState &archiverState);

    tempo_utils::Status define_call(
        lyric_importer::CallImport *callImport,
        lyric_assembler::CallSymbol *callSymbol,
        const std::string &importHash,
        lyric_assembler::NamespaceSymbol *targetNamespace,
        SymbolReferenceSet &symbolReferenceSet,
        ArchiverState &archiverState);

    tempo_utils::Status put_pending_proc(
        lyric_importer::CallImport *callImport,
        lyric_assembler::ProcHandle *procHandle,
        SymbolReferenceSet &symbolReferenceSet,
        ArchiverState &archiverState);
}

#endif // LYRIC_ARCHIVER_COPY_CALL_H
