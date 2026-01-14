#ifndef LYRIC_SCHEMA_BUILD_SCHEMA_H
#define LYRIC_SCHEMA_BUILD_SCHEMA_H

#include <array>

#include <tempo_schema/schema.h>
#include <tempo_schema/schema_namespace.h>
#include <tempo_schema/schema_resource.h>

namespace lyric_schema {

    class LyricBuildNs : public tempo_schema::SchemaNs {
    public:
        constexpr LyricBuildNs() : tempo_schema::SchemaNs("dev.zuri.ns:lyric_build") {};
    };
    constexpr LyricBuildNs kLyricBuildNs;

    enum class LyricBuildId {
        ModuleLocation,
        ContentUrl,
        ContentType,
        EntryEnum,
        Generation,
        InstallPath,
        TaskHash,
        TaskParams,
        RuntimeLibDirectory,
        LibDirectory,
        NUM_IDS,
    };

    constexpr tempo_schema::SchemaProperty<LyricBuildNs,LyricBuildId>
    kLyricBuildModuleLocationProperty(
        &kLyricBuildNs, LyricBuildId::ModuleLocation, "ModuleLocation", tempo_schema::PropertyType::kString);

    constexpr tempo_schema::SchemaProperty<LyricBuildNs,LyricBuildId>
    kLyricBuildContentUrlProperty(
        &kLyricBuildNs, LyricBuildId::ContentUrl, "ContentUrl", tempo_schema::PropertyType::kString);

    constexpr tempo_schema::SchemaProperty<LyricBuildNs,LyricBuildId>
    kLyricBuildContentTypeProperty(
        &kLyricBuildNs, LyricBuildId::ContentType, "ContentType", tempo_schema::PropertyType::kString);

    constexpr tempo_schema::SchemaProperty<LyricBuildNs,LyricBuildId>
        kLyricBuildEntryEnumProperty(
        &kLyricBuildNs, LyricBuildId::EntryEnum, "EntryEnum", tempo_schema::PropertyType::kUInt32);

    constexpr tempo_schema::SchemaProperty<LyricBuildNs,LyricBuildId>
    kLyricBuildGenerationProperty(
        &kLyricBuildNs, LyricBuildId::Generation, "Generation", tempo_schema::PropertyType::kString);

    constexpr tempo_schema::SchemaProperty<LyricBuildNs,LyricBuildId>
    kLyricBuildInstallPathProperty(
        &kLyricBuildNs, LyricBuildId::InstallPath, "InstallPath", tempo_schema::PropertyType::kString);

    constexpr tempo_schema::SchemaProperty<LyricBuildNs,LyricBuildId>
    kLyricBuildTaskHashProperty(
        &kLyricBuildNs, LyricBuildId::TaskHash, "TaskHash", tempo_schema::PropertyType::kString);

    constexpr tempo_schema::SchemaProperty<LyricBuildNs,LyricBuildId>
    kLyricBuildTaskParamsProperty(
        &kLyricBuildNs, LyricBuildId::TaskParams, "TaskParams", tempo_schema::PropertyType::kString);

    constexpr tempo_schema::SchemaProperty<LyricBuildNs,LyricBuildId>
    kLyricBuildRuntimeLibDirectoryProperty(
        &kLyricBuildNs, LyricBuildId::RuntimeLibDirectory, "RuntimeLibDirectory", tempo_schema::PropertyType::kString);

    constexpr tempo_schema::SchemaProperty<LyricBuildNs,LyricBuildId>
    kLyricBuildLibDirectoryProperty(
        &kLyricBuildNs, LyricBuildId::LibDirectory, "LibDirectory", tempo_schema::PropertyType::kString);

    constexpr std::array<
        const tempo_schema::SchemaResource<LyricBuildNs,LyricBuildId> *,
        static_cast<std::size_t>(LyricBuildId::NUM_IDS)>
    kLyricBuildResources = {
        &kLyricBuildModuleLocationProperty,
        &kLyricBuildContentUrlProperty,
        &kLyricBuildContentTypeProperty,
        &kLyricBuildEntryEnumProperty,
        &kLyricBuildGenerationProperty,
        &kLyricBuildInstallPathProperty,
        &kLyricBuildTaskHashProperty,
        &kLyricBuildTaskParamsProperty,
        &kLyricBuildRuntimeLibDirectoryProperty,
        &kLyricBuildLibDirectoryProperty,
    };

    constexpr tempo_schema::SchemaVocabulary<LyricBuildNs, LyricBuildId>
    kLyricBuildVocabulary(&kLyricBuildNs, &kLyricBuildResources);
};

#endif // LYRIC_SCHEMA_BUILD_SCHEMA_H
