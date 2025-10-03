#ifndef LYRIC_ARCHIVER_COPY_PROC_H
#define LYRIC_ARCHIVER_COPY_PROC_H

#include <lyric_assembler/proc_handle.h>
#include <lyric_object/bytecode_iterator.h>

namespace lyric_archiver {

    tempo_utils::Status copy_proc(
        const lyric_common::ModuleLocation &objectLocation,
        const lyric_object::LyricObject &object,
        const lyric_common::ModuleLocation &pluginLocation,
        std::shared_ptr<const lyric_runtime::AbstractPlugin> plugin,
        const lyric_object::ProcInfo &procInfo,
        const absl::flat_hash_map<lyric_common::SymbolUrl,lyric_assembler::AbstractSymbol *> *copiedSymbols,
        lyric_assembler::ProcHandle *procHandle,
        lyric_assembler::ObjectState *state);
}

#endif // LYRIC_ARCHIVER_COPY_PROC_H
