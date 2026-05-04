#ifndef LYRIC_TYPING_INTERNAL_CHECK_PLACEHOLDER_H
#define LYRIC_TYPING_INTERNAL_CHECK_PLACEHOLDER_H
#include "lyric_assembler/object_state.h"

namespace lyric_typing::internal {

    tempo_utils::Status check_placeholder(
        const lyric_object::TemplateParameter &tp,
        const lyric_common::TypeDef &arg,
        lyric_assembler::ObjectState *objectState);
}

#endif // LYRIC_TYPING_INTERNAL_CHECK_PLACEHOLDER_H
