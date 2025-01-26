#ifndef LYRIC_ARCHIVER_COPY_INSTANCE_H
#define LYRIC_ARCHIVER_COPY_INSTANCE_H

#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/object_root.h>
#include <lyric_common/symbol_url.h>
#include <lyric_importer/instance_import.h>

#include "archiver_state.h"

namespace lyric_archiver {

    tempo_utils::Result<lyric_assembler::InstanceSymbol *> copy_instance(
        lyric_importer::InstanceImport *instanceImport,
        const std::string &importHash,
        lyric_assembler::NamespaceSymbol *targetNamespace,
        SymbolReferenceSet &symbolReferenceSet,
        ArchiverState &archiverState);
}

#endif // LYRIC_ARCHIVER_COPY_INSTANCE_H