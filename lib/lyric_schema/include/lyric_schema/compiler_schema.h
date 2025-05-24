#ifndef LYRIC_SCHEMA_COMPILER_SCHEMA_H
#define LYRIC_SCHEMA_COMPILER_SCHEMA_H

#include <array>

#include <tempo_schema/schema.h>
#include <tempo_schema/schema_namespace.h>

namespace lyric_schema {

    class LyricCompilerNs : public tempo_schema::SchemaNs {
    public:
        constexpr LyricCompilerNs() : tempo_schema::SchemaNs("dev.zuri.ns:lyric_compiler") {};
    };
    constexpr LyricCompilerNs kLyricCompilerNs;

    enum class LyricCompilerId {

        // Compiler classes

        PushResult,                 // push result macro
        PopResult,                  // pop result macro

        // Compiler properties

        NUM_IDS,                    // must be last
    };

    constexpr tempo_schema::SchemaClass<LyricCompilerNs,LyricCompilerId> kLyricCompilerPushResultClass(
        &kLyricCompilerNs, LyricCompilerId::PushResult, "PushResult");
    constexpr tempo_schema::SchemaClass<LyricCompilerNs,LyricCompilerId> kLyricCompilerPopResultClass(
        &kLyricCompilerNs, LyricCompilerId::PopResult, "PopResult");

    constexpr std::array<
        const tempo_schema::SchemaResource<LyricCompilerNs,LyricCompilerId> *,
        static_cast<std::size_t>(LyricCompilerId::NUM_IDS)>
        kLyricCompilerResources = {
        &kLyricCompilerPushResultClass,
        &kLyricCompilerPopResultClass,
    };

    constexpr tempo_schema::SchemaVocabulary<LyricCompilerNs, LyricCompilerId>
    kLyricCompilerVocabulary(&kLyricCompilerNs, &kLyricCompilerResources);
};

#endif // LYRIC_SCHEMA_COMPILER_SCHEMA_H
