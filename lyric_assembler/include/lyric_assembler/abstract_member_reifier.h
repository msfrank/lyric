#ifndef LYRIC_ASSEMBLER_ABSTRACT_MEMBER_REIFIER_H
#define LYRIC_ASSEMBLER_ABSTRACT_MEMBER_REIFIER_H

#include <vector>

#include <absl/container/flat_hash_map.h>

#include <lyric_common/symbol_url.h>

#include "assembler_result.h"
#include "assembler_types.h"
#include "field_symbol.h"

namespace lyric_assembler {

    class AbstractMemberReifier {

    public:
        virtual ~AbstractMemberReifier() = default;

        virtual bool isValid() const = 0;

        virtual tempo_utils::Result<DataReference> reifyMember(
            const std::string &name,
            const FieldSymbol *fieldSymbol) = 0;
    };
}

#endif // LYRIC_ASSEMBLER_ABSTRACT_MEMBER_REIFIER_H
