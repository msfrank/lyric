#ifndef LYRIC_TYPING_VALIDATE_SUBTYPE_H
#define LYRIC_TYPING_VALIDATE_SUBTYPE_H
#include "lyric_assembler/abstract_symbol.h"
#include "lyric_assembler/object_state.h"
#include "lyric_common/type_def.h"

namespace lyric_typing {

    tempo_utils::Status validate_subtype(
        const lyric_common::TypeDef &subType,
        lyric_assembler::AbstractSymbol *symbol,
        lyric_assembler::ObjectState *state);
}

#endif // LYRIC_TYPING_VALIDATE_SUBTYPE_H
