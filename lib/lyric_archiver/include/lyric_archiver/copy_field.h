#ifndef LYRIC_ARCHIVER_COPY_FIELD_H
#define LYRIC_ARCHIVER_COPY_FIELD_H

#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/object_root.h>
#include <lyric_common/symbol_url.h>
#include <lyric_importer/field_import.h>

#include "archiver_state.h"

namespace lyric_archiver {

    tempo_utils::Result<lyric_assembler::FieldSymbol *> copy_field(
        lyric_importer::FieldImport *fieldImport,
        const std::string &importHash,
        lyric_assembler::NamespaceSymbol *targetNamespace,
        SymbolReferenceSet &symbolReferenceSet,
        ArchiverState &archiverState);
}

#endif // LYRIC_ARCHIVER_COPY_FIELD_H