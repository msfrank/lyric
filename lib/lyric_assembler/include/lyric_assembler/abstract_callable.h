#ifndef LYRIC_ASSEMBLER_ABSTRACT_CALLABLE_H
#define LYRIC_ASSEMBLER_ABSTRACT_CALLABLE_H

#include <lyric_common/symbol_url.h>
#include <lyric_object/object_types.h>

#include "abstract_callsite_reifier.h"
#include "abstract_placement.h"
#include "assembler_types.h"

namespace lyric_assembler {

    class BlockHandle;
    class CodeFragment;
    class TemplateHandle;

    class AbstractCallable : public AbstractPlacement {
    public:

        virtual tempo_utils::Result<lyric_common::TypeDef> invoke(
            BlockHandle *block,
            const AbstractCallsiteReifier &reifier,
            CodeFragment *fragment) = 0;
    };
}

#endif // LYRIC_ASSEMBLER_ABSTRACT_CALLABLE_H