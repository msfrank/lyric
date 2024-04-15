#ifndef LYRIC_RUNTIME_LITERAL_CELL_H
#define LYRIC_RUNTIME_LITERAL_CELL_H

#include <absl/strings/string_view.h>
#include <unicode/ustring.h>

#include <tempo_utils/log_message.h>

namespace lyric_runtime {

    enum class LiteralCellType : uint8_t {
        INVALID,
        NIL,
        PRESENT,
        BOOL,
        I64,
        DBL,
        CHAR32,
        UTF8,
    };

    struct LiteralCell {

        LiteralCell();
        explicit LiteralCell(bool b);
        explicit LiteralCell(int64_t i64);
        explicit LiteralCell(double dbl);
        explicit LiteralCell(UChar32 chr);
        explicit LiteralCell(std::string_view sv);
        LiteralCell(const LiteralCell &other);
        LiteralCell(LiteralCell &&other) noexcept;

        LiteralCell& operator=(const LiteralCell &other);
        LiteralCell& operator=(LiteralCell &&other) noexcept;

        bool isValid() const;

        bool operator==(const LiteralCell &other) const;
        bool operator!=(const LiteralCell &other) const;

        std::string toString() const;

        static LiteralCell nil();
        static LiteralCell present();

        LiteralCellType type;
        union {
            bool b;
            int64_t i64;
            double dbl;
            UChar32 chr;
            struct {
                const char *data;
                int32_t size;
            } utf8;
        } literal;
    };

    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const LiteralCell &cell);

    template <typename H>
    H AbslHashValue(H h, const LiteralCell &cell) {
        switch (cell.type) {
            case LiteralCellType::INVALID:
            case LiteralCellType::NIL:
            case LiteralCellType::PRESENT:
                return H::combine(std::move(h), cell.type);
            case LiteralCellType::BOOL:
                return H::combine(std::move(h), cell.type, cell.literal.b);
            case LiteralCellType::I64:
                return H::combine(std::move(h), cell.type, cell.literal.i64);
            case LiteralCellType::DBL:
                return H::combine(std::move(h), cell.type, cell.literal.dbl);
            case LiteralCellType::CHAR32:
                return H::combine(std::move(h), cell.type, cell.literal.chr);
            case LiteralCellType::UTF8: {
                auto state = H::combine(std::move(h), cell.type);
                return H::combine_contiguous(std::move(state), cell.literal.utf8.data, cell.literal.utf8.size);
            }
        }
        TU_UNREACHABLE();
    }
}

#endif // LYRIC_RUNTIME_LITERAL_CELL_H
