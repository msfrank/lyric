#ifndef LYRIC_SCHEMA_PACKAGING_SCHEMA_H
#define LYRIC_SCHEMA_PACKAGING_SCHEMA_H

#include <array>

#include <tempo_utils/schema.h>

namespace lyric_schema {

    class LyricPackagingNs : public tempo_utils::SchemaNs {
    public:
        constexpr LyricPackagingNs() : tempo_utils::SchemaNs("dev.zuri.ns:lyric_packaging") {};
    };
    constexpr LyricPackagingNs kLyricPackagingNs;

    enum class LyricPackagingId {
        CreateTime,    // Timestamp when the content was created, expressed as seconds since the epoch
        ExpiryTime,    // Timestamp after which the content is considered expired, expressed as seconds since the epoch
        ContentType,   // Content media type expressed as a string
        MainLocation,
        NUM_IDS,
    };

    constexpr tempo_utils::SchemaProperty<LyricPackagingNs,LyricPackagingId>
    kLyricPackagingCreateTimeProperty(
        &kLyricPackagingNs, LyricPackagingId::CreateTime, "CreateTime", tempo_utils::PropertyType::kUInt64);

    constexpr tempo_utils::SchemaProperty<LyricPackagingNs,LyricPackagingId>
    kLyricPackagingExpiryTimeProperty(
        &kLyricPackagingNs, LyricPackagingId::ExpiryTime, "ExpiryTime", tempo_utils::PropertyType::kUInt64);

    constexpr tempo_utils::SchemaProperty<LyricPackagingNs,LyricPackagingId>
    kLyricPackagingContentTypeProperty(
        &kLyricPackagingNs, LyricPackagingId::ContentType, "ContentType", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<LyricPackagingNs,LyricPackagingId>
    kLyricPackagingMainLocationProperty(
        &kLyricPackagingNs, LyricPackagingId::MainLocation, "MainLocation", tempo_utils::PropertyType::kString);
}

#endif // LYRIC_SCHEMA_PACKAGING_SCHEMA_H