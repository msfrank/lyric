#ifndef LYRIC_ASSEMBLER_LITERAL_CACHE_H
#define LYRIC_ASSEMBLER_LITERAL_CACHE_H

#include <tempo_utils/result.h>

#include "literal_handle.h"

namespace lyric_assembler {

    class LiteralCache {
    public:
        LiteralCache();
        ~LiteralCache();

        tempo_utils::Result<LiteralHandle *> getOrMakeLiteral(std::string_view utf8);
        tempo_utils::Result<LiteralHandle *> getOrMakeLiteral(std::span<const tu_uint8> bytes);
        tempo_utils::Result<LiteralHandle *> getOrMakeLiteral(const tempo_utils::Url &url);

        std::vector<LiteralHandle *>::const_iterator literalsBegin() const;
        std::vector<LiteralHandle *>::const_iterator literalsEnd() const;
        int numLiterals() const;

    private:
        std::vector<LiteralHandle *> m_literals;
        absl::flat_hash_map<std::string_view, tu_uint32> m_stringcache;
    };
}

#endif // LYRIC_ASSEMBLER_LITERAL_CACHE_H
