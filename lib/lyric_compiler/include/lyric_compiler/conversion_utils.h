#ifndef LYRIC_COMPILER_CONVERSION_UTILS_H
#define LYRIC_COMPILER_CONVERSION_UTILS_H

#include <lyric_assembler/block_handle.h>

namespace lyric_compiler {

    tempo_utils::Result<bool> convert_operand(
        const lyric_common::TypeDef &fromType,
        const lyric_common::TypeDef &intoType,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment);
}

#endif // LYRIC_COMPILER_CONVERSION_UTILS_H
