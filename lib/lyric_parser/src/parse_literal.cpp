
#include <lyric_parser/parse_literal.h>
#include <tempo_utils/unicode.h>

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