#ifndef LYRIC_SCHEMA_PARSER_SCHEMA_H
#define LYRIC_SCHEMA_PARSER_SCHEMA_H

#include <array>

#include <tempo_schema/schema_namespace.h>
#include <tempo_schema/schema_resource.h>

namespace lyric_schema {

    class LyricParserNs : public tempo_schema::SchemaNs {
    public:
        constexpr LyricParserNs() : tempo_schema::SchemaNs("dev.zuri.ns:lyric-parser-1") {};
    };
    constexpr LyricParserNs kLyricParserNs;

    enum class LyricParserId {
        NodeOffset,
        AttrOffset,
        LineNumber,
        ColumnNumber,
        FileOffset,
        TextSpan,
        Identifier,
        NUM_IDS,
    };

    constexpr tempo_schema::SchemaProperty<LyricParserNs,LyricParserId>
    kLyricParserNodeOffsetProperty(
        &kLyricParserNs, LyricParserId::NodeOffset, "NodeOffset", tempo_schema::PropertyType::kUInt32);

    constexpr tempo_schema::SchemaProperty<LyricParserNs,LyricParserId>
    kLyricParserAttrOffsetProperty(
        &kLyricParserNs, LyricParserId::AttrOffset, "AttrOffset", tempo_schema::PropertyType::kUInt32);

    constexpr tempo_schema::SchemaProperty<LyricParserNs,LyricParserId>
    kLyricParserLineNumberProperty(
        &kLyricParserNs, LyricParserId::LineNumber, "LineNumber", tempo_schema::PropertyType::kUInt32);

    constexpr tempo_schema::SchemaProperty<LyricParserNs,LyricParserId>
    kLyricParserColumnNumberProperty(
        &kLyricParserNs, LyricParserId::ColumnNumber, "ColumnNumber", tempo_schema::PropertyType::kUInt32);

    constexpr tempo_schema::SchemaProperty<LyricParserNs,LyricParserId>
    kLyricParserFileOffsetProperty(
        &kLyricParserNs, LyricParserId::FileOffset, "FileOffset", tempo_schema::PropertyType::kUInt32);

    constexpr tempo_schema::SchemaProperty<LyricParserNs,LyricParserId>
    kLyricParserTextSpanProperty(
        &kLyricParserNs, LyricParserId::TextSpan, "TextSpan", tempo_schema::PropertyType::kUInt32);

    constexpr tempo_schema::SchemaProperty<LyricParserNs,LyricParserId>
    kLyricParserIdentifierProperty(
        &kLyricParserNs, LyricParserId::Identifier, "Identifier", tempo_schema::PropertyType::kString);
}

#endif // LYRIC_SCHEMA_PARSER_SCHEMA_H