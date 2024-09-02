#ifndef LYRIC_ASSEMBLER_LITERAL_HANDLE_H
#define LYRIC_ASSEMBLER_LITERAL_HANDLE_H

#include <absl/container/flat_hash_map.h>

#include <lyric_common/symbol_url.h>
#include <lyric_runtime/literal_cell.h>

#include "assembler_types.h"

namespace lyric_assembler {

    class LiteralHandle {

    public:
        LiteralHandle();
        LiteralHandle(bool b);
        LiteralHandle(tu_int64 i64);
        LiteralHandle(double dbl);
        LiteralHandle(UChar32 chr);
        LiteralHandle(std::shared_ptr<const std::string> str);

        //LiteralAddress getAddress() const;

        lyric_runtime::LiteralCellType getType() const;
        bool getBool() const;
        int64_t getInt64() const;
        double getDouble() const;
        UChar32 getUChar32() const;
        std::shared_ptr<const std::string> getUtf8() const;

    private:
        lyric_runtime::LiteralCellType m_type;
        union {
            bool b;
            tu_int64 i64;
            double dbl;
            UChar32 chr;
        } m_value;
        std::shared_ptr<const std::string> m_str;
    };
}

#endif // LYRIC_ASSEMBLER_LITERAL_HANDLE_H
