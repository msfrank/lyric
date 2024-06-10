#ifndef LYRIC_ASSEMBLER_ABSTRACT_CALLSITE_REIFIER_H
#define LYRIC_ASSEMBLER_ABSTRACT_CALLSITE_REIFIER_H

#include <lyric_common/symbol_url.h>

#include "assembler_types.h"

namespace lyric_assembler {

    class AbstractCallsiteReifier {

    public:
        virtual ~AbstractCallsiteReifier() = default;

        //virtual bool isValid() const = 0;
        //virtual lyric_common::TypeDef getArgument(int index) const = 0;
        //virtual std::vector<lyric_common::TypeDef> getArguments() const = 0;
        //virtual int numArguments() const = 0;

        virtual size_t numReifiedArguments() const = 0;

        virtual tempo_utils::Status reifyNextArgument(const lyric_common::TypeDef &argumentType) = 0;

        virtual tempo_utils::Result<lyric_common::TypeDef> reifyNextContext() = 0;

        virtual tempo_utils::Result<lyric_common::TypeDef> reifyResult(
            const lyric_common::TypeDef &returnType) const = 0;
    };
}

#endif // LYRIC_ASSEMBLER_ABSTRACT_CALLSITE_REIFIER_H
