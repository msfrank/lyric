#ifndef LYRIC_ASSEMBLER_LITERAL_CACHE_H
#define LYRIC_ASSEMBLER_LITERAL_CACHE_H

#include "assembler_tracer.h"
#include "assembler_types.h"
#include "literal_handle.h"

namespace lyric_assembler {

    class LiteralCache {
    public:
        explicit LiteralCache(AssemblerTracer *tracer);
        ~LiteralCache();

        tempo_utils::Result<LiteralAddress> makeLiteralNil();
        tempo_utils::Result<LiteralAddress> makeLiteralBool(bool b);
        tempo_utils::Result<LiteralAddress> makeLiteralInteger(int64_t i64);
        tempo_utils::Result<LiteralAddress> makeLiteralFloat(double dbl);
        tempo_utils::Result<LiteralAddress> makeLiteralChar(UChar32 chr);
        tempo_utils::Result<LiteralAddress> makeLiteralUtf8(const std::string &utf8);

        std::vector<LiteralHandle *>::const_iterator literalsBegin() const;
        std::vector<LiteralHandle *>::const_iterator literalsEnd() const;

    private:
        AssemblerTracer *m_tracer;
        std::vector<LiteralHandle *> m_literals;
        absl::flat_hash_map<lyric_runtime::LiteralCell, LiteralAddress> m_literalcache;
        absl::flat_hash_map<std::string, LiteralAddress> m_stringcache;
    };
}

#endif // LYRIC_ASSEMBLER_LITERAL_CACHE_H
