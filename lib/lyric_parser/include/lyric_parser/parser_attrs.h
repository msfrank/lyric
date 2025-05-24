#ifndef LYRIC_PARSER_PARSER_SCHEMA_H
#define LYRIC_PARSER_PARSER_SCHEMA_H

#include <lyric_schema/parser_schema.h>
#include <tempo_schema/attr_serde.h>

namespace lyric_parser {

    extern const tempo_schema::Int64Attr kLyricParserNodeOffset;
    extern const tempo_schema::Int64Attr kLyricParserAttrOffset;
    extern const tempo_schema::Int64Attr kLyricParserLineNumber;
    extern const tempo_schema::Int64Attr kLyricParserColumnNumber;
    extern const tempo_schema::Int64Attr kLyricParserFileOffset;
    extern const tempo_schema::Int64Attr kLyricParserTextSpan;
    extern const tempo_schema::StringAttr kLyricParserIdentifier;
}

#endif // LYRIC_PARSER_PARSER_SCHEMA_H