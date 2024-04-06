#ifndef LYRIC_TYPING_RESOLVE_TEMPLATE_H
#define LYRIC_TYPING_RESOLVE_TEMPLATE_H

#include "tempo_utils/result.h"
#include "lyric_assembler/assembler_types.h"
#include "lyric_parser/node_walker.h"
#include "lyric_assembler/block_handle.h"

namespace lyric_typing {

    tempo_utils::Result<lyric_assembler::TemplateSpec>
    resolve_template(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_assembler::AssemblyState *state);


    tempo_utils::Result<std::pair<lyric_object::BoundType,lyric_common::TypeDef>>
    resolve_bound(
        const lyric_common::TypeDef &placeholderType,
        lyric_assembler::AssemblyState *state);
}

#endif // LYRIC_TYPING_RESOLVE_TEMPLATE_H