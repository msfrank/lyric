#ifndef LYRIC_PARSER_PARSER_SCHEMA_H
#define LYRIC_PARSER_PARSER_SCHEMA_H

#include <lyric_schema/parser_schema.h>
#include <tempo_utils/attr.h>

namespace lyric_parser {

    extern const tempo_utils::Int64Attr kLyricParserNodeOffset;
    extern const tempo_utils::Int64Attr kLyricParserAttrOffset;
    extern const tempo_utils::Int64Attr kLyricParserLineNumber;
    extern const tempo_utils::Int64Attr kLyricParserColumnNumber;
    extern const tempo_utils::Int64Attr kLyricParserFileOffset;
    extern const tempo_utils::Int64Attr kLyricParserTextSpan;
    extern const tempo_utils::StringAttr kLyricParserIdentifier;
}

#endif // LYRIC_PARSER_PARSER_SCHEMA_H