#ifndef LYRIC_SCHEMA_ASSEMBLER_SCHEMA_H
#define LYRIC_SCHEMA_ASSEMBLER_SCHEMA_H

#include <array>

#include <tempo_utils/schema.h>

namespace lyric_schema {

    class LyricAssemblerNs : public tempo_utils::SchemaNs {
    public:
        constexpr LyricAssemblerNs() : tempo_utils::SchemaNs("dev.zuri.ns:lyric_assembler") {};
    };
    constexpr LyricAssemblerNs kLyricAssemblerNs;

    enum class LyricAssemblerId {
        Trap,
        NUM_IDS,
    };

    constexpr tempo_utils::SchemaClass<LyricAssemblerNs,LyricAssemblerId> kLyricAssemblerTrapClass(
        &kLyricAssemblerNs, LyricAssemblerId::Trap, "Trap");

    constexpr std::array<
        const tempo_utils::SchemaResource<LyricAssemblerNs,LyricAssemblerId> *,
        static_cast<std::size_t>(LyricAssemblerId::NUM_IDS)>
    kLyricAssemblerResources = {
        &kLyricAssemblerTrapClass,
    };

    constexpr tempo_utils::SchemaVocabulary<LyricAssemblerNs, LyricAssemblerId>
    kLyricAssemblerVocabulary(&kLyricAssemblerNs, &kLyricAssemblerResources);
};

#endif // LYRIC_SCHEMA_ASSEMBLER_SCHEMA_H
