#ifndef LYRIC_SCHEMA_COMPILER_SCHEMA_H
#define LYRIC_SCHEMA_COMPILER_SCHEMA_H

#include <array>

#include <tempo_utils/schema.h>

namespace lyric_schema {

    class LyricCompilerNs : public tempo_utils::SchemaNs {
    public:
        constexpr LyricCompilerNs() : tempo_utils::SchemaNs("dev.zuri.ns:lyric_compiler") {};
    };
    constexpr LyricCompilerNs kLyricCompilerNs;

    enum class LyricCompilerId {

        // Compiler classes

        PushResult,                 // push result macro
        PopResult,                  // pop result macro

        // Compiler properties

        NUM_IDS,                    // must be last
    };

    constexpr tempo_utils::SchemaClass<LyricCompilerNs,LyricCompilerId> kLyricCompilerPushResultClass(
        &kLyricCompilerNs, LyricCompilerId::PushResult, "PushResult");
    constexpr tempo_utils::SchemaClass<LyricCompilerNs,LyricCompilerId> kLyricCompilerPopResultClass(
        &kLyricCompilerNs, LyricCompilerId::PopResult, "PopResult");

    constexpr std::array<
        const tempo_utils::SchemaResource<LyricCompilerNs,LyricCompilerId> *,
        static_cast<std::size_t>(LyricCompilerId::NUM_IDS)>
        kLyricCompilerResources = {
        &kLyricCompilerPushResultClass,
        &kLyricCompilerPopResultClass,
    };

    constexpr tempo_utils::SchemaVocabulary<LyricCompilerNs, LyricCompilerId>
    kLyricCompilerVocabulary(&kLyricCompilerNs, &kLyricCompilerResources);
};

#endif // LYRIC_SCHEMA_COMPILER_SCHEMA_H
