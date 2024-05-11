#ifndef LYRIC_PARSER_PARSE_LITERAL_H
#define LYRIC_PARSER_PARSE_LITERAL_H

#include <unicode/umachine.h>

#include <tempo_utils/result.h>

#include "parse_result.h"
#include "parser_types.h"

namespace lyric_parser {

    tempo_utils::Result<int64_t> parse_integer_literal(
        std::string_view literalValue,
        BaseType baseType);

    tempo_utils::Result<double> parse_float_literal(
        std::string_view literalValue,
        BaseType baseType,
        NotationType notationType);

    tempo_utils::Result<UChar32> parse_char_literal(std::string_view literalValue);

    tempo_utils::Result<std::string> parse_string_literal(
        std::string_view literalValue,
        tu_int32 maxUnescapedSize = -1);
}

#endif // LYRIC_PARSER_PARSE_LITERAL_H
