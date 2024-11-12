#ifndef LYRIC_COMPILER_MATCH_UTILS_H
#define LYRIC_COMPILER_MATCH_UTILS_H

#include <tempo_utils/status.h>

#include "match_handler.h"

namespace lyric_compiler {

    tempo_utils::Result<lyric_common::TypeDef>
    compile_predicate(
        const lyric_assembler::DataReference &targetRef,
        const lyric_common::TypeDef &predicateType,
        lyric_assembler::CodeFragment *fragment,
        lyric_assembler::BlockHandle *block,
        lyric_compiler::CompilerScanDriver *driver);

    tempo_utils::Status check_match_is_exhaustive(
        Match *match,
        lyric_assembler::BlockHandle *block,
        CompilerScanDriver *driver);

}

#endif // LYRIC_COMPILER_MATCH_UTILS_H
