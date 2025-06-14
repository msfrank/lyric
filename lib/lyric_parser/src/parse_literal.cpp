
#include <absl/strings/charconv.h>

#include <lyric_parser/parse_literal.h>
#include <tempo_utils/unicode.h>

tempo_utils::Result<tu_int64>
lyric_parser::parse_integer_literal(std::string_view literalValue, BaseType baseType)
{
    int base;
    switch (baseType) {
        case lyric_parser::BaseType::Binary:
            base = 2;
            break;
        case lyric_parser::BaseType::Octal:
            base = 8;
            break;
        case lyric_parser::BaseType::Decimal:
            base = 10;
            break;
        case lyric_parser::BaseType::Hex:
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
    if (baseType == BaseType::Decimal && notationType == NotationType::Fixed) {
        fmt = absl::chars_format::fixed;
    } else if (baseType == BaseType::Decimal && notationType == NotationType::Scientific) {
        fmt = absl::chars_format::scientific;
    } else if (baseType == BaseType::Hex && notationType == NotationType::Fixed) {
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
tempo_utils::Result<char32_t>
lyric_parser::parse_char_literal(std::string_view literalValue)
{
    // special case: string is empty so no escaping necessary
    if (literalValue.empty())
        return char32_t{0};

    auto unescaped = tempo_utils::unescape_utf8(literalValue);
    if (tempo_utils::get_utf8_length(unescaped) == 0)
        return ParseStatus::forCondition(
            ParseCondition::kSyntaxError, "empty char literal");

    return tempo_utils::get_utf8_char(unescaped, 0);
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
        return std::string{};

    auto unescaped = tempo_utils::unescape_utf8(literalValue);
    auto unescapedSize = tempo_utils::get_utf8_length(unescaped);
    if (maxUnescapedSize < unescapedSize)
        return ParseStatus::forCondition(ParseCondition::kSyntaxError,
            "utf8 literal is too large (size was {} code units)", unescapedSize);

    return unescaped;
}