
#include <absl/strings/charconv.h>
#include <unicode/ustring.h>

#include <lyric_parser/parse_literal.h>

tempo_utils::Result<tu_int64>
lyric_parser::parse_integer_literal(std::string_view literalValue, BaseType baseType)
{
    int base;
    switch (baseType) {
        case lyric_parser::BaseType::BINARY:
            base = 2;
            break;
        case lyric_parser::BaseType::OCTAL:
            base = 8;
            break;
        case lyric_parser::BaseType::DECIMAL:
            base = 10;
            break;
        case lyric_parser::BaseType::HEX:
            base = 16;
            break;
        default:
            return ParseStatus::forCondition(ParseCondition::kSyntaxError, "invalid integer base type");
    }

    char *endptr = nullptr;
    int64_t i64 = strtoll(literalValue.data(), &endptr, base);
    if (*endptr != '\0')
        return ParseStatus::forCondition(ParseCondition::kSyntaxError,
            "invalid integer literal '{}'", literalValue);
    if (errno == ERANGE)
        return ParseStatus::forCondition(ParseCondition::kSyntaxError,
            "integer literal '{}' is out of range", literalValue);

    return i64;
}

tempo_utils::Result<double>
lyric_parser::parse_float_literal(
    std::string_view literalValue,
    BaseType baseType,
    NotationType notationType)
{
    const char *first = literalValue.data();
    if (first[0] == '+')
        first++;

    absl::chars_format fmt;
    if (baseType == BaseType::DECIMAL && notationType == NotationType::FIXED) {
        fmt = absl::chars_format::fixed;
    } else if (baseType == BaseType::DECIMAL && notationType == NotationType::SCIENTIFIC) {
        fmt = absl::chars_format::scientific;
    } else if (baseType == BaseType::HEX && notationType == NotationType::FIXED) {
        fmt = absl::chars_format::hex;
    } else {
        return ParseStatus::forCondition(
            ParseCondition::kSyntaxError, "invalid float base type");
    }

    double dbl;
    auto conversion = absl::from_chars(first, literalValue.data() + literalValue.size(), dbl, fmt);
    if (conversion.ptr == first)
        return ParseStatus::forCondition(ParseCondition::kSyntaxError,
            "invalid float literal '{}'", literalValue);
    if (conversion.ec != std::errc()) {
        switch (conversion.ec) {
            case std::errc::result_out_of_range:
                return ParseStatus::forCondition(ParseCondition::kSyntaxError,
                    "float literal '{}' is out of range", literalValue);
            case std::errc::invalid_argument:
            default:
                return ParseStatus::forCondition(ParseCondition::kSyntaxError,
                    "invalid float literal '{}'", literalValue);
        }
    }

    return dbl;
}

/**
 * Unescape the given `literalValue` and return it as a UTF-32 character.
 *
 * @param literalValue The character literal to parse.
 * \return The unescaped character encoded as UTF-32.
 */
tempo_utils::Result<UChar32>
lyric_parser::parse_char_literal(std::string_view literalValue)
{
    // perform unescape on the string
    int32_t bufsize = literalValue.size() + 1;
    UChar buf[bufsize];
    auto nwritten = u_unescape(literalValue.data(), buf, bufsize);

    UChar32 chr;
    U16_GET(buf, 0, 0, nwritten, chr);

    return chr;
}

/**
 * Unescape the given `literalValue` and return it as a UTF-8 encoded string.
 *
 * @param literalValue The string literal to parse.
 * @param maxUnescapedSize If non-negative then it is an error if the unescaped string literal exceeds maxUnescapedSize.
 * \return The unescaped string encoded as UTF-8.
 */
tempo_utils::Result<std::string>
lyric_parser::parse_string_literal(std::string_view literalValue, tu_int32 maxUnescapedSize)
{
    // special case: string is empty so no escaping necessary
    if (literalValue.empty())
        return std::string(literalValue);

    // perform preflight to determine size of unescaped buffer
    int32_t unescapedSize = -1;
    unescapedSize = u_unescape(literalValue.data(), nullptr, 0);
    if (unescapedSize < 0)
        return ParseStatus::forCondition(ParseCondition::kParseInvariant,
            "preflight failed for utf8 literal");
    if (maxUnescapedSize >= 0 && unescapedSize > maxUnescapedSize)
        return ParseStatus::forCondition(ParseCondition::kSyntaxError,
            "utf8 literal is too large (size was {} code units)", unescapedSize);

    // perform unescape on the string
    UChar unescaped[unescapedSize];
    auto nwritten = u_unescape(literalValue.data(), unescaped, unescapedSize);
    if (nwritten <= 0)
        return ParseStatus::forCondition(ParseCondition::kParseInvariant,
            "unescape failed for utf8 literal");

    // perform preflight to determine size of dst buffer
    int32_t dstSize = -1;
    UErrorCode err = U_ZERO_ERROR;
    u_strToUTF8(nullptr, 0, &dstSize, unescaped, unescapedSize, &err);
    if (err != U_ZERO_ERROR && err != U_BUFFER_OVERFLOW_ERROR)
        return ParseStatus::forCondition(ParseCondition::kParseInvariant,
            "preflight failed for utf8 literal");
    if (dstSize > (1024 * 1024 * 4))    // FIXME: make this a defined constant somewhere
        return ParseStatus::forCondition(ParseCondition::kSyntaxError,
            "utf8 literal is too large");

    err = U_ZERO_ERROR;

    // convert utf16 back to utf8
    std::string dst;
    dst.resize(dstSize);    // NOTE: dstSize is in utf8 code units, which is a single byte
    u_strToUTF8(dst.data(), dstSize, &dstSize, unescaped, unescapedSize, &err);
    if (err != U_ZERO_ERROR && err != U_STRING_NOT_TERMINATED_WARNING)
        return ParseStatus::forCondition(ParseCondition::kParseInvariant,
            "invalid utf8 literal '{}'", literalValue);

    return dst;
}