#ifndef LYRIC_TYPING_RESOLVE_TEMPLATE_H
#define LYRIC_TYPING_RESOLVE_TEMPLATE_H

#include <lyric_parser/node_walker.h>
#include <tempo_utils/result.h>

#include "typing_types.h"

namespace lyric_typing {

    tempo_utils::Result<TemplateSpec>
    resolve_template(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_assembler::ObjectState *state);


    tempo_utils::Result<std::pair<lyric_object::BoundType,lyric_common::TypeDef>>
    resolve_bound(
        const lyric_common::TypeDef &placeholderType,
        lyric_assembler::ObjectState *state);
}

#endif // LYRIC_TYPING_RESOLVE_TEMPLATE_H