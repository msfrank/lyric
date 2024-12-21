#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_ACTIONS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_ACTIONS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_state.h"
#include "../object_writer.h"

#include "write_symbols.h"

namespace lyric_assembler::internal {

    tempo_utils::Status touch_action(
        const ActionSymbol *actionSymbol,
        const ObjectState *objectState,
        ObjectWriter &writer);

    using ActionsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::ActionDescriptor>>>;

    tempo_utils::Status write_actions(
        const std::vector<const ActionSymbol *> &actions,
        const ObjectWriter &writer,
        flatbuffers::FlatBufferBuilder &buffer,
        ActionsOffset &actionsOffset);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_ACTIONS_H
