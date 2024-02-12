#ifndef LYRIC_ASSEMBLER_ABSTRACT_RESOLVER_H
#define LYRIC_ASSEMBLER_ABSTRACT_RESOLVER_H

#include <lyric_common/type_def.h>
#include <lyric_parser/assignable.h>
#include <tempo_utils/result.h>

namespace lyric_assembler {

    class AbstractResolver {
    public:
        virtual ~AbstractResolver() = default;

        /**
         *
         * @param assignableSpec
         * @return
         */
        virtual tempo_utils::Result<lyric_common::TypeDef> resolveAssignable(
            const lyric_parser::Assignable &assignableSpec) = 0;
    };
}

#endif // LYRIC_ASSEMBLER_ABSTRACT_RESOLVER_H
