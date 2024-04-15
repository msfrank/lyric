
#include <absl/strings/substitute.h>

#include <lyric_runtime/literal_cell.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

lyric_runtime::LiteralCell::LiteralCell()
{
    type = LiteralCellType::INVALID;
}

lyric_runtime::LiteralCell::LiteralCell(bool b)
{
    type = LiteralCellType::BOOL;
    literal.b = b;
}

lyric_runtime::LiteralCell::LiteralCell(int64_t i64)
{
    type = LiteralCellType::I64;
    literal.i64 = i64;
}

lyric_runtime::LiteralCell::LiteralCell(double dbl)
{
    type = LiteralCellType::DBL;
    literal.dbl = dbl;
}


lyric_runtime::LiteralCell::LiteralCell(UChar32 chr)
{
    type = LiteralCellType::CHAR32;
    literal.chr = chr;
}

lyric_runtime::LiteralCell::LiteralCell(std::string_view sv)
{
    type = LiteralCellType::UTF8;
    literal.utf8.data = sv.data();
    literal.utf8.size = static_cast<tu_int32>(sv.size());
}

lyric_runtime::LiteralCell::LiteralCell(const LiteralCell &other) : LiteralCell()
{
    switch (other.type) {
        case LiteralCellType::INVALID:
        case LiteralCellType::NIL:
            type = other.type;
            break;
        case LiteralCellType::BOOL:
            type = other.type;
            literal.b = other.literal.b;
            break;
        case LiteralCellType::I64:
            type = other.type;
            literal.i64 = other.literal.i64;
            break;
        case LiteralCellType::DBL:
            type = other.type;
            literal.dbl = other.literal.dbl;
            break;
        case LiteralCellType::CHAR32:
            type = other.type;
            literal.chr = other.literal.chr;
            break;
        case LiteralCellType::UTF8:
            type = other.type;
            literal.utf8.data = other.literal.utf8.data;
            literal.utf8.size = other.literal.utf8.size;
            break;
        default:
            TU_UNREACHABLE();
    }
}

lyric_runtime::LiteralCell::LiteralCell(LiteralCell &&other) noexcept : LiteralCell()
{
    type = other.type;
    switch (other.type) {
        case LiteralCellType::INVALID:
        case LiteralCellType::NIL:
            break;
        case LiteralCellType::BOOL:
            literal.b = other.literal.b;
            break;
        case LiteralCellType::I64:
            literal.i64 = other.literal.i64;
            break;
        case LiteralCellType::DBL:
            literal.dbl = other.literal.dbl;
            break;
        case LiteralCellType::CHAR32:
            literal.chr = other.literal.chr;
            break;
        case LiteralCellType::UTF8:
            literal.utf8.data = other.literal.utf8.data;
            literal.utf8.size = other.literal.utf8.size;
            break;
        default:
            TU_UNREACHABLE();
    }
    other.type = LiteralCellType::INVALID;
}

lyric_runtime::LiteralCell&
lyric_runtime::LiteralCell::operator=(const LiteralCell &other)
{
    switch (other.type) {
        case LiteralCellType::INVALID:
        case LiteralCellType::NIL:
            type = other.type;
            break;
        case LiteralCellType::BOOL:
            type = other.type;
            literal.b = other.literal.b;
            break;
        case LiteralCellType::I64:
            type = other.type;
            literal.i64 = other.literal.i64;
            break;
        case LiteralCellType::DBL:
            type = other.type;
            literal.dbl = other.literal.dbl;
            break;
        case LiteralCellType::CHAR32:
            type = other.type;
            literal.chr = other.literal.chr;
            break;
        case LiteralCellType::UTF8:
            type = other.type;
            literal.utf8.data = other.literal.utf8.data;
            literal.utf8.size = other.literal.utf8.size;
            break;
        default:
            TU_UNREACHABLE();
    }
    return *this;
}

lyric_runtime::LiteralCell&
lyric_runtime::LiteralCell::operator=(LiteralCell &&other) noexcept
{
    type = other.type;
    switch (other.type) {
        case LiteralCellType::INVALID:
        case LiteralCellType::NIL:
            break;
        case LiteralCellType::BOOL:
            literal.b = other.literal.b;
            break;
        case LiteralCellType::I64:
            literal.i64 = other.literal.i64;
            break;
        case LiteralCellType::DBL:
            literal.dbl = other.literal.dbl;
            break;
        case LiteralCellType::CHAR32:
            literal.chr = other.literal.chr;
            break;
        case LiteralCellType::UTF8:
            literal.utf8.data = other.literal.utf8.data;
            literal.utf8.size = other.literal.utf8.size;
            break;
        default:
            TU_UNREACHABLE();
    }
    other.type = LiteralCellType::INVALID;
    return *this;
}

bool
lyric_runtime::LiteralCell::isValid() const
{
    switch (type) {
        case LiteralCellType::NIL:
        case LiteralCellType::BOOL:
        case LiteralCellType::I64:
        case LiteralCellType::DBL:
        case LiteralCellType::CHAR32:
        case LiteralCellType::UTF8:
            return true;
        default:
            break;
    }
    return false;
}

bool
lyric_runtime::LiteralCell::operator==(const LiteralCell &other) const
{
    if (type != other.type)
        return false;
    switch (type) {
        case LiteralCellType::INVALID:
        case LiteralCellType::NIL:
            return true;
        case LiteralCellType::BOOL:
            return literal.b == other.literal.b;
        case LiteralCellType::I64:
            return literal.i64 == other.literal.i64;
        case LiteralCellType::DBL:
            return literal.dbl == other.literal.dbl;
        case LiteralCellType::CHAR32:
            return literal.chr == other.literal.chr;
        case LiteralCellType::UTF8:
            if (literal.utf8.size != other.literal.utf8.size)
                return false;
            return !strncmp(literal.utf8.data, other.literal.utf8.data, literal.utf8.size);
        default:
            break;
    }
    return false;
}

bool
lyric_runtime::LiteralCell::operator!=(const LiteralCell &other) const
{
    return !(*this == other);
}

lyric_runtime::LiteralCell
lyric_runtime::LiteralCell::nil()
{
    LiteralCell cell;
    cell.type = LiteralCellType::NIL;
    return cell;
}

std::string
lyric_runtime::LiteralCell::toString() const
{
    switch (type) {
        case LiteralCellType::INVALID:
            return "LiteralCell(INVALID)";
        case LiteralCellType::NIL:
            return "LiteralCell(Nil)";
        case LiteralCellType::BOOL:
            return literal.b? "LiteralCell(bool=true)" : "LiteralCell(bool=false)";
        case LiteralCellType::I64:
            return absl::Substitute("LiteralCell(integer=$0)", literal.i64);
        case LiteralCellType::DBL:
            return absl::Substitute("LiteralCell(float=$0)", literal.dbl);
        case LiteralCellType::CHAR32:
            return absl::Substitute("LiteralCell(char=$0)",
                tempo_utils::convert_to_utf8(literal.chr));
        case LiteralCellType::UTF8:
            return absl::Substitute("LiteralCell(utf8=\"$0\")",
                std::string_view(literal.utf8.data, literal.utf8.size));
        default:
            break;
    }
    TU_UNREACHABLE();
}

tempo_utils::LogMessage&&
lyric_runtime::operator<<(tempo_utils::LogMessage &&message, const lyric_runtime::LiteralCell &cell)
{
    std::forward<tempo_utils::LogMessage>(message) << cell.toString();
    return std::move(message);
}
