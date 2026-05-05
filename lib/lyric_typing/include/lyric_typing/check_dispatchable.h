#ifndef LYRIC_TYPING_COMPARE_DISPATCHABLE_H
#define LYRIC_TYPING_COMPARE_DISPATCHABLE_H

#include <lyric_assembler/abstract_symbol.h>
#include <lyric_assembler/assembler_types.h>
#include <lyric_assembler/object_state.h>

namespace lyric_typing {

    tempo_utils::Status check_dispatchable(
        const lyric_assembler::ParameterPack &toParameters,
        const lyric_common::TypeDef &toResult,
        const lyric_assembler::ParameterPack &fromParameters,
        const lyric_common::TypeDef &fromResult,
        lyric_assembler::ObjectState *state);
}

#endif // LYRIC_TYPING_COMPARE_DISPATCHABLE_H
