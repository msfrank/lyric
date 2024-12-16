#ifndef LYRIC_ASSEMBER_INTERNAL_WRITE_INSTANCES_H
#define LYRIC_ASSEMBER_INTERNAL_WRITE_INSTANCES_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_state.h"
#include "../object_writer.h"

#include "symbol_table.h"

namespace lyric_assembler::internal {

    tempo_utils::Status touch_instance(
        const InstanceSymbol *instanceSymbol,
        const ObjectState *objectState,
        ObjectWriter &writer);

    using InstancesOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::InstanceDescriptor>>>;

    tempo_utils::Status write_instances(
        const std::vector<const InstanceSymbol *> &instances,
        const ObjectWriter &writer,
        flatbuffers::FlatBufferBuilder &buffer,
        InstancesOffset &instancesOffset,
        SymbolTable &symbolTable);
}

#endif // LYRIC_ASSEMBER_INTERNAL_WRITE_INSTANCES_H
