#ifndef LYRIC_ASSEMBLER_INTERNAL_IMPORT_PROC_H
#define LYRIC_ASSEMBLER_INTERNAL_IMPORT_PROC_H

#include <lyric_object/bytecode_iterator.h>

#include "../proc_handle.h"

namespace lyric_assembler::internal {

    tempo_utils::Status import_proc(
        const lyric_common::ModuleLocation &location,
        const lyric_object::LyricObject &object,
        const lyric_common::SymbolUrl &activation,
        lyric_object::BytecodeIterator it,
        std::unique_ptr<ProcHandle> &procHandle,
        ObjectState *state);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_IMPORT_PROC_H
