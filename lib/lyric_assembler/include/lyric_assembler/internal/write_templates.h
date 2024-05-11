#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_TEMPLATES_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_TEMPLATES_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../type_cache.h"

namespace lyric_assembler::internal {

    using TemplatesOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::TemplateDescriptor>>>;

    tempo_utils::Status write_templates(
        TypeCache *typeCache,
        flatbuffers::FlatBufferBuilder &buffer,
        TemplatesOffset &templatesOffset);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_TEMPLATES_H
