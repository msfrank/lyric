#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_BINDINGS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_BINDINGS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_state.h"
#include "../object_writer.h"

#include "write_symbols.h"

namespace lyric_assembler::internal {

    tempo_utils::Status touch_binding(
        const BindingSymbol *bindingSymbol,
        const ObjectState *objectState,
        ObjectWriter &writer);

    using BindingsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::BindingDescriptor>>>;

    tempo_utils::Status write_bindings(
        const std::vector<const BindingSymbol *> &bindings,
        const ObjectWriter &writer,
        flatbuffers::FlatBufferBuilder &buffer,
        BindingsOffset &bindingsOffset);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_BINDINGS_H
