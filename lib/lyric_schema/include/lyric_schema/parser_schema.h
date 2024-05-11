#ifndef LYRIC_SCHEMA_PARSER_SCHEMA_H
#define LYRIC_SCHEMA_PARSER_SCHEMA_H

#include "tempo_utils/schema.h"

namespace lyric_schema {

    class LyricParserNs : public tempo_utils::SchemaNs {
    public:
        constexpr LyricParserNs() : tempo_utils::SchemaNs("dev.zuri.ns:lyric-parser-1") {};
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

    constexpr tempo_utils::SchemaProperty<LyricParserNs,LyricParserId>
    kLyricParserNodeOffsetProperty(
        &kLyricParserNs, LyricParserId::NodeOffset, "NodeOffset", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricParserNs,LyricParserId>
    kLyricParserAttrOffsetProperty(
        &kLyricParserNs, LyricParserId::AttrOffset, "AttrOffset", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricParserNs,LyricParserId>
    kLyricParserLineNumberProperty(
        &kLyricParserNs, LyricParserId::LineNumber, "LineNumber", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricParserNs,LyricParserId>
    kLyricParserColumnNumberProperty(
        &kLyricParserNs, LyricParserId::ColumnNumber, "ColumnNumber", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricParserNs,LyricParserId>
    kLyricParserFileOffsetProperty(
        &kLyricParserNs, LyricParserId::FileOffset, "FileOffset", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricParserNs,LyricParserId>
    kLyricParserTextSpanProperty(
        &kLyricParserNs, LyricParserId::TextSpan, "TextSpan", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricParserNs,LyricParserId>
    kLyricParserIdentifierProperty(
        &kLyricParserNs, LyricParserId::Identifier, "Identifier", tempo_utils::PropertyType::kString);
}

#endif // LYRIC_SCHEMA_PARSER_SCHEMA_H