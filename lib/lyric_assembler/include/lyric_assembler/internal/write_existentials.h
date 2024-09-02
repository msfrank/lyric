#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_EXISTENTIALS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_EXISTENTIALS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_state.h"
#include "../object_writer.h"

namespace lyric_assembler::internal {

    tempo_utils::Status touch_existential(
        const ExistentialSymbol *existentialSymbol,
        const ObjectState *objectState,
        ObjectWriter &writer);

}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_EXISTENTIALS_H
