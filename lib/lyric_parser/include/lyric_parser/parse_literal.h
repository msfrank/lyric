#ifndef LYRIC_PARSER_PARSE_LITERAL_H
#define LYRIC_PARSER_PARSE_LITERAL_H

#include <tempo_utils/result.h>

#include "parse_result.h"
#include "parser_types.h"

namespace lyric_parser {

    tempo_utils::Result<char32_t> parse_char_literal(std::string_view literalValue);

    tempo_utils::Result<std::string> parse_string_literal(
        std::string_view literalValue,
        tu_int32 maxUnescapedSize = -1);
}

#endif // LYRIC_PARSER_PARSE_LITERAL_H
