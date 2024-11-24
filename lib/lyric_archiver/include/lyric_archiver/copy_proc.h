#ifndef LYRIC_ARCHIVER_COPY_PROC_H
#define LYRIC_ARCHIVER_COPY_PROC_H

#include <lyric_assembler/proc_handle.h>
#include <lyric_object/bytecode_iterator.h>

namespace lyric_archiver {

    tempo_utils::Status copy_proc(
        const lyric_common::ModuleLocation &location,
        const lyric_object::LyricObject &object,
        const lyric_object::ProcHeader &header,
        lyric_object::BytecodeIterator it,
        lyric_assembler::ProcHandle *procHandle,
        lyric_assembler::ObjectState *state);
}

#endif // LYRIC_ARCHIVER_COPY_PROC_H
