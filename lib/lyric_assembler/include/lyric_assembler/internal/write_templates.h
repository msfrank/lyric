#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_TEMPLATES_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_TEMPLATES_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_writer.h"
#include "../type_cache.h"

namespace lyric_assembler::internal {

    tempo_utils::Status touch_template(
        const TemplateHandle *templateHandle,
        const ObjectState *objectState,
        ObjectWriter &writer);

    using TemplatesOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::TemplateDescriptor>>>;

    tempo_utils::Status write_templates(
        const std::vector<const TemplateHandle *> &templates,
        const ObjectWriter &writer,
        flatbuffers::FlatBufferBuilder &buffer,
        TemplatesOffset &templatesOffset);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_TEMPLATES_H
