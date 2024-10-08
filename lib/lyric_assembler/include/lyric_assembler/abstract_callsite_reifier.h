#ifndef LYRIC_ASSEMBLER_ABSTRACT_CALLSITE_REIFIER_H
#define LYRIC_ASSEMBLER_ABSTRACT_CALLSITE_REIFIER_H

#include <lyric_common/symbol_url.h>
#include <tempo_utils/result.h>

#include "assembler_types.h"

namespace lyric_assembler {

    class AbstractCallsiteReifier {

    public:
        virtual ~AbstractCallsiteReifier() = default;

        virtual size_t numReifiedArguments() const = 0;

        virtual tempo_utils::Status reifyNextArgument(const lyric_common::TypeDef &argumentType) = 0;

        virtual tempo_utils::Result<lyric_common::TypeDef> reifyNextContext() = 0;

        virtual tempo_utils::Result<lyric_common::TypeDef> reifyResult(
            const lyric_common::TypeDef &returnType) const = 0;
    };
}

#endif // LYRIC_ASSEMBLER_ABSTRACT_CALLSITE_REIFIER_H
