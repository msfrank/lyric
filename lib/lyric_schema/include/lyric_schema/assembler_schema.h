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

        // Assembler classes

        Trap,                       // trap macro

        // Assembler properties

        TrapNumber,                 // integer identifying the trap in the plugin

        NUM_IDS,                    // must be last
    };

    constexpr tempo_utils::SchemaClass<LyricAssemblerNs,LyricAssemblerId> kLyricAssemblerTrapClass(
        &kLyricAssemblerNs, LyricAssemblerId::Trap, "Trap");

    constexpr tempo_utils::SchemaProperty<LyricAssemblerNs,LyricAssemblerId>
        kLyricAssemblerTrapNumberProperty(
        &kLyricAssemblerNs, LyricAssemblerId::TrapNumber, "TrapNumber", tempo_utils::PropertyType::kUInt32);

    constexpr std::array<
        const tempo_utils::SchemaResource<LyricAssemblerNs,LyricAssemblerId> *,
        static_cast<std::size_t>(LyricAssemblerId::NUM_IDS)>
    kLyricAssemblerResources = {
        &kLyricAssemblerTrapClass,
        &kLyricAssemblerTrapNumberProperty,
    };

    constexpr tempo_utils::SchemaVocabulary<LyricAssemblerNs, LyricAssemblerId>
    kLyricAssemblerVocabulary(&kLyricAssemblerNs, &kLyricAssemblerResources);
};

#endif // LYRIC_SCHEMA_ASSEMBLER_SCHEMA_H
