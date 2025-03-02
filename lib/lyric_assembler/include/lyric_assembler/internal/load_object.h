#ifndef LYRIC_ASSEMBLER_INTERNAL_LOAD_OBJECT_H
#define LYRIC_ASSEMBLER_INTERNAL_LOAD_OBJECT_H

#include "../object_state.h"

namespace lyric_assembler::internal {

    tempo_utils::Result<lyric_common::ModuleLocation> find_system_bootstrap(
        std::shared_ptr<lyric_importer::ModuleImport> moduleImport,
        ObjectState *state);

    tempo_utils::Status load_object_symbols(
        std::shared_ptr<lyric_importer::ModuleImport> moduleImport,
        ObjectState *state);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_LOAD_OBJECT_H
