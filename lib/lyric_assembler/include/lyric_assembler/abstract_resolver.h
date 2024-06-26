#ifndef LYRIC_ASSEMBLER_ABSTRACT_RESOLVER_H
#define LYRIC_ASSEMBLER_ABSTRACT_RESOLVER_H

#include <lyric_common/type_def.h>
#include <tempo_utils/result.h>

namespace lyric_assembler {

    class AbstractResolver {
    public:
        virtual ~AbstractResolver() = default;

        virtual tempo_utils::Result<lyric_common::TypeDef> resolveSingular(
            const lyric_common::SymbolPath &typePath,
            const std::vector<lyric_common::TypeDef> &typeArguments) = 0;
    };
}

#endif // LYRIC_ASSEMBLER_ABSTRACT_RESOLVER_H
