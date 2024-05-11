#ifndef LYRIC_SCHEMA_BUILD_SCHEMA_H
#define LYRIC_SCHEMA_BUILD_SCHEMA_H

#include <tempo_utils/schema.h>

namespace lyric_schema {

    class LyricBuildNs : public tempo_utils::SchemaNs {
    public:
        constexpr LyricBuildNs() : tempo_utils::SchemaNs("dev.zuri.ns:lyric_build") {};
    };
    constexpr LyricBuildNs kLyricBuildNs;

    enum class LyricBuildId {
        AssemblyLocation,
        ContentUrl,
        EntryEnum,
        Generation,
        InstallPath,
        TaskHash,
        TaskParams,
        NUM_IDS,
    };

    constexpr tempo_utils::SchemaProperty<LyricBuildNs,LyricBuildId>
    kLyricBuildAssemblyLocationProperty(
        &kLyricBuildNs, LyricBuildId::AssemblyLocation, "AssemblyLocation", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<LyricBuildNs,LyricBuildId>
    kLyricBuildContentUrlProperty(
        &kLyricBuildNs, LyricBuildId::ContentUrl, "ContentUrl", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<LyricBuildNs,LyricBuildId>
        kLyricBuildEntryEnumProperty(
        &kLyricBuildNs, LyricBuildId::EntryEnum, "EntryEnum", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricBuildNs,LyricBuildId>
    kLyricBuildGenerationProperty(
        &kLyricBuildNs, LyricBuildId::Generation, "Generation", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<LyricBuildNs,LyricBuildId>
    kLyricBuildInstallPathProperty(
        &kLyricBuildNs, LyricBuildId::InstallPath, "InstallPath", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<LyricBuildNs,LyricBuildId>
    kLyricBuildTaskHashProperty(
        &kLyricBuildNs, LyricBuildId::TaskHash, "TaskHash", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<LyricBuildNs,LyricBuildId>
    kLyricBuildTaskParamsProperty(
        &kLyricBuildNs, LyricBuildId::TaskParams, "TaskParams", tempo_utils::PropertyType::kString);

    constexpr std::array<
        const tempo_utils::SchemaResource<LyricBuildNs,LyricBuildId> *,
        static_cast<std::size_t>(LyricBuildId::NUM_IDS)>
    kLyricBuildResources = {
        &kLyricBuildAssemblyLocationProperty,
        &kLyricBuildContentUrlProperty,
        &kLyricBuildEntryEnumProperty,
        &kLyricBuildGenerationProperty,
        &kLyricBuildInstallPathProperty,
        &kLyricBuildTaskHashProperty,
        &kLyricBuildTaskParamsProperty,
    };

    constexpr tempo_utils::SchemaVocabulary<LyricBuildNs, LyricBuildId>
    kLyricBuildVocabulary(&kLyricBuildNs, &kLyricBuildResources);
};

#endif // LYRIC_SCHEMA_BUILD_SCHEMA_H
