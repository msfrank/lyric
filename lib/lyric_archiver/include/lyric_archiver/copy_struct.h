#ifndef LYRIC_ARCHIVER_COPY_STRUCT_H
#define LYRIC_ARCHIVER_COPY_STRUCT_H

#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/object_root.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_common/symbol_url.h>
#include <lyric_importer/struct_import.h>

#include "archiver_state.h"

namespace lyric_archiver {

    tempo_utils::Result<lyric_assembler::StructSymbol *> copy_struct(
        lyric_importer::StructImport *structImport,
        const std::string &importHash,
        lyric_assembler::NamespaceSymbol *targetNamespace,
        SymbolReferenceSet &symbolReferenceSet,
        ArchiverState &archiverState);
}

#endif // LYRIC_ARCHIVER_COPY_STRUCT_H