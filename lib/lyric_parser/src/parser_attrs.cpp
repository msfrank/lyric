
#include <lyric_parser/parser_attrs.h>

const tempo_schema::Int64Attr lyric_parser::kLyricParserNodeOffset(
    &lyric_schema::kLyricParserNodeOffsetProperty);

const tempo_schema::Int64Attr lyric_parser::kLyricParserAttrOffset(
    &lyric_schema::kLyricParserAttrOffsetProperty);

const tempo_schema::Int64Attr lyric_parser::kLyricParserLineNumber(
    &lyric_schema::kLyricParserLineNumberProperty);

const tempo_schema::Int64Attr lyric_parser::kLyricParserColumnNumber(
    &lyric_schema::kLyricParserColumnNumberProperty);

const tempo_schema::Int64Attr lyric_parser::kLyricParserFileOffset(
    &lyric_schema::kLyricParserFileOffsetProperty);

const tempo_schema::Int64Attr lyric_parser::kLyricParserTextSpan(
    &lyric_schema::kLyricParserTextSpanProperty);

const tempo_schema::StringAttr lyric_parser::kLyricParserIdentifier(
    &lyric_schema::kLyricParserTextSpanProperty);
