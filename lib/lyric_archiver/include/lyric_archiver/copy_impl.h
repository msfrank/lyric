#ifndef LYRIC_ARCHIVER_COPY_IMPL_H
#define LYRIC_ARCHIVER_COPY_IMPL_H

#include <lyric_assembler/impl_handle.h>
#include <lyric_importer/impl_import.h>

#include "archiver_state.h"

namespace lyric_archiver {

    tempo_utils::Status copy_impl(
        lyric_importer::ImplImport *implImport,
        lyric_assembler::ImplHandle *implHandle,
        SymbolReferenceSet &symbolReferenceSet,
        ArchiverState &archiverState);

    tempo_utils::Status define_extension(
        const std::string &name,
        lyric_importer::CallImport *callImport,
        lyric_assembler::ImplHandle *implHandle,
        SymbolReferenceSet &symbolReferenceSet,
        ArchiverState &archiverState);
}

#endif // LYRIC_ARCHIVER_COPY_IMPL_H
