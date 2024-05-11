#ifndef LYRIC_SCHEMA_LITERAL_SCHEMA_H
#define LYRIC_SCHEMA_LITERAL_SCHEMA_H

#include <cstdint>
#include <array>

#include <tempo_utils/schema.h>

namespace lyric_schema {

    class LyricLiteralNs : public tempo_utils::SchemaNs {
    public:
        constexpr LyricLiteralNs() : tempo_utils::SchemaNs("io.fathomdata:ns:literals-1") {};
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

    constexpr tempo_utils::SchemaProperty<LyricLiteralNs,LyricLiteralId>
    kLyricLiteralNilProperty(
        &kLyricLiteralNs, LyricLiteralId::Nil, "Nil", tempo_utils::PropertyType::kNil);

    constexpr tempo_utils::SchemaProperty<LyricLiteralNs,LyricLiteralId>
    kLyricLiteralBoolProperty(
        &kLyricLiteralNs, LyricLiteralId::Bool, "Bool", tempo_utils::PropertyType::kBool);

    constexpr tempo_utils::SchemaProperty<LyricLiteralNs,LyricLiteralId>
    kLyricLiteralLongProperty(
        &kLyricLiteralNs, LyricLiteralId::Long, "Long", tempo_utils::PropertyType::kInt64);

    constexpr tempo_utils::SchemaProperty<LyricLiteralNs,LyricLiteralId>
    kLyricLiteralDoubleProperty(
        &kLyricLiteralNs, LyricLiteralId::Double, "Double", tempo_utils::PropertyType::kFloat64);

    constexpr tempo_utils::SchemaProperty<LyricLiteralNs,LyricLiteralId>
    kLyricLiteralStringProperty(
        &kLyricLiteralNs, LyricLiteralId::String, "String", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaClass<LyricLiteralNs,LyricLiteralId>
    kLyricLiteralSeqClass(
        &kLyricLiteralNs, LyricLiteralId::Seq, "Seq");

    constexpr tempo_utils::SchemaClass<LyricLiteralNs,LyricLiteralId>
    kLyricLiteralMapClass(
        &kLyricLiteralNs, LyricLiteralId::Map, "Map");

    constexpr std::array<
        const tempo_utils::SchemaResource<LyricLiteralNs,LyricLiteralId> *,
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

    constexpr tempo_utils::SchemaVocabulary<LyricLiteralNs, LyricLiteralId>
    kLyricLiteralVocabulary(&kLyricLiteralNs, &kLyricLiteralResources);
};

#endif // LYRIC_SCHEMA_LITERAL_SCHEMA_H