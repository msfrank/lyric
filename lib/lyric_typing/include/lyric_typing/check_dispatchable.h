#ifndef LYRIC_TYPING_COMPARE_DISPATCHABLE_H
#define LYRIC_TYPING_COMPARE_DISPATCHABLE_H

#include <lyric_assembler/abstract_symbol.h>
#include <lyric_assembler/assembler_types.h>
#include <lyric_assembler/object_state.h>

namespace lyric_typing {

    tempo_utils::Status check_dispatchable(
        lyric_assembler::AbstractSymbol *symbol,
        const lyric_assembler::ParameterPack &parameterPack,
        const lyric_common::TypeDef &returnType,
        lyric_assembler::ObjectState *state);
}

#endif // LYRIC_TYPING_COMPARE_DISPATCHABLE_H
