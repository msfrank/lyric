#ifndef LYRIC_ASSEMBLER_LITERAL_CACHE_H
#define LYRIC_ASSEMBLER_LITERAL_CACHE_H

#include <tempo_utils/result.h>

#include "assembler_types.h"
#include "literal_handle.h"

namespace lyric_assembler {

    class LiteralCache {
    public:
        LiteralCache();
        ~LiteralCache();

        tempo_utils::Result<LiteralHandle *> makeNil();
        tempo_utils::Result<LiteralHandle *> makeUndef();
        tempo_utils::Result<LiteralHandle *> makeBool(bool b);
        tempo_utils::Result<LiteralHandle *> makeInteger(tu_int64 i64);
        tempo_utils::Result<LiteralHandle *> makeFloat(double dbl);
        tempo_utils::Result<LiteralHandle *> makeChar(UChar32 chr);
        tempo_utils::Result<LiteralHandle *> makeUtf8(const std::string &utf8);

        std::vector<LiteralHandle *>::const_iterator literalsBegin() const;
        std::vector<LiteralHandle *>::const_iterator literalsEnd() const;

    private:
        std::vector<LiteralHandle *> m_literals;
        absl::flat_hash_map<lyric_runtime::LiteralCell, tu_uint32> m_literalcache;
        absl::flat_hash_map<std::string, tu_uint32> m_stringcache;
    };
}

#endif // LYRIC_ASSEMBLER_LITERAL_CACHE_H
