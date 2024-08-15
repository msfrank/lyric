#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_ACTIONS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_ACTIONS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_state.h"

namespace lyric_assembler::internal {

    using ActionsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::ActionDescriptor>>>;

    tempo_utils::Status write_actions(
        const ObjectState *objectState,
        flatbuffers::FlatBufferBuilder &buffer,
        ActionsOffset &actionsOffset,
        std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_ACTIONS_H
