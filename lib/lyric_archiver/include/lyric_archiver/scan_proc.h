#ifndef LYRIC_ARCHIVER_SCAN_PROC_H
#define LYRIC_ARCHIVER_SCAN_PROC_H

#include <lyric_assembler/proc_handle.h>
#include <lyric_importer/call_import.h>
#include <lyric_object/bytecode_iterator.h>

#include "archiver_state.h"

namespace lyric_archiver {

    tempo_utils::Status scan_proc(
        const lyric_common::ModuleLocation &location,
        const lyric_object::LyricObject &object,
        const lyric_object::ProcInfo &procInfo,
        SymbolReferenceSet &symbolReferenceSet,
        ArchiverState &archiverState);
}

#endif // LYRIC_ARCHIVER_SCAN_PROC_H
