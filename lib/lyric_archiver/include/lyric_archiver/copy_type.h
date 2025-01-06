#ifndef LYRIC_ARCHIVER_COPY_TYPE_H
#define LYRIC_ARCHIVER_COPY_TYPE_H

#include <lyric_assembler/object_root.h>
#include <lyric_assembler/type_handle.h>
#include <lyric_common/symbol_url.h>
#include <lyric_importer/type_import.h>

#include "archiver_state.h"

namespace lyric_archiver {

    tempo_utils::Result<lyric_assembler::TypeHandle *> copy_type(
        lyric_importer::TypeImport *typeImport,
        const std::string &importHash,
        lyric_assembler::NamespaceSymbol *targetNamespace,
        SymbolReferenceSet &symbolReferenceSet,
        ArchiverState &archiverState);
}

#endif // LYRIC_ARCHIVER_COPY_TYPE_H
