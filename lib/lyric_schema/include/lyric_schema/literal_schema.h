#ifndef LYRIC_SCHEMA_LITERAL_SCHEMA_H
#define LYRIC_SCHEMA_LITERAL_SCHEMA_H

#include <array>

#include <tempo_schema/schema.h>
#include <tempo_schema/schema_namespace.h>

namespace lyric_schema {

    class LyricLiteralNs : public tempo_schema::SchemaNs {
    public:
        constexpr LyricLiteralNs() : tempo_schema::SchemaNs("io.fathomdata:ns:literals-1") {};
    };
    constexpr LyricLiteralNs kLyricLiteralNs;

    enum class LyricLiteralId {
        Nil,
        Bool,
        Long,
        Double,
        String,
        Seq,
        Map,
        NUM_IDS,
    };

    constexpr tempo_schema::SchemaProperty<LyricLiteralNs,LyricLiteralId>
    kLyricLiteralNilProperty(
        &kLyricLiteralNs, LyricLiteralId::Nil, "Nil", tempo_schema::PropertyType::kNil);

    constexpr tempo_schema::SchemaProperty<LyricLiteralNs,LyricLiteralId>
    kLyricLiteralBoolProperty(
        &kLyricLiteralNs, LyricLiteralId::Bool, "Bool", tempo_schema::PropertyType::kBool);

    constexpr tempo_schema::SchemaProperty<LyricLiteralNs,LyricLiteralId>
    kLyricLiteralLongProperty(
        &kLyricLiteralNs, LyricLiteralId::Long, "Long", tempo_schema::PropertyType::kInt64);

    constexpr tempo_schema::SchemaProperty<LyricLiteralNs,LyricLiteralId>
    kLyricLiteralDoubleProperty(
        &kLyricLiteralNs, LyricLiteralId::Double, "Double", tempo_schema::PropertyType::kFloat64);

    constexpr tempo_schema::SchemaProperty<LyricLiteralNs,LyricLiteralId>
    kLyricLiteralStringProperty(
        &kLyricLiteralNs, LyricLiteralId::String, "String", tempo_schema::PropertyType::kString);

    constexpr tempo_schema::SchemaClass<LyricLiteralNs,LyricLiteralId>
    kLyricLiteralSeqClass(
        &kLyricLiteralNs, LyricLiteralId::Seq, "Seq");

    constexpr tempo_schema::SchemaClass<LyricLiteralNs,LyricLiteralId>
    kLyricLiteralMapClass(
        &kLyricLiteralNs, LyricLiteralId::Map, "Map");

    constexpr std::array<
        const tempo_schema::SchemaResource<LyricLiteralNs,LyricLiteralId> *,
        static_cast<std::size_t>(LyricLiteralId::NUM_IDS)>
    kLyricLiteralResources = {
        &kLyricLiteralNilProperty,
        &kLyricLiteralBoolProperty,
        &kLyricLiteralLongProperty,
        &kLyricLiteralDoubleProperty,
        &kLyricLiteralStringProperty,
        &kLyricLiteralSeqClass,
        &kLyricLiteralMapClass,
    };

    constexpr tempo_schema::SchemaVocabulary<LyricLiteralNs, LyricLiteralId>
    kLyricLiteralVocabulary(&kLyricLiteralNs, &kLyricLiteralResources);
};

#endif // LYRIC_SCHEMA_LITERAL_SCHEMA_H