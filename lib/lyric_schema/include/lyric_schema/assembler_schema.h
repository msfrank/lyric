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

        Trap,                       // invoke trap with the given trap number
        AllocatorTrap,              // use the given trap as allocator

        // Assembler properties

        TrapNumber,                 // integer identifying the trap in the plugin
        AllocatorTrapNumber,        // integer identifying the allocator trap in the plugin
        DefinitionSymbolPath,       //

        NUM_IDS,                    // must be last
    };

    constexpr tempo_utils::SchemaClass<LyricAssemblerNs,LyricAssemblerId> kLyricAssemblerTrapClass(
        &kLyricAssemblerNs, LyricAssemblerId::Trap, "Trap");

    constexpr tempo_utils::SchemaClass<LyricAssemblerNs,LyricAssemblerId> kLyricAssemblerAllocatorTrapClass(
        &kLyricAssemblerNs, LyricAssemblerId::AllocatorTrap, "AllocatorTrap");

    constexpr tempo_utils::SchemaProperty<LyricAssemblerNs,LyricAssemblerId>
        kLyricAssemblerTrapNumberProperty(
        &kLyricAssemblerNs, LyricAssemblerId::TrapNumber, "TrapNumber", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricAssemblerNs,LyricAssemblerId>
        kLyricAssemblerAllocatorTrapNumberProperty(
        &kLyricAssemblerNs, LyricAssemblerId::AllocatorTrapNumber, "AllocatorTrapNumber", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<LyricAssemblerNs,LyricAssemblerId>
        kLyricAssemblerDefinitionSymbolPathProperty(
        &kLyricAssemblerNs, LyricAssemblerId::DefinitionSymbolPath, "DefinitionSymbolPath", tempo_utils::PropertyType::kString);

    constexpr std::array<
        const tempo_utils::SchemaResource<LyricAssemblerNs,LyricAssemblerId> *,
        static_cast<std::size_t>(LyricAssemblerId::NUM_IDS)>
    kLyricAssemblerResources = {
        &kLyricAssemblerTrapClass,
        &kLyricAssemblerAllocatorTrapClass,
        &kLyricAssemblerTrapNumberProperty,
        &kLyricAssemblerAllocatorTrapNumberProperty,
        &kLyricAssemblerDefinitionSymbolPathProperty,
    };

    constexpr tempo_utils::SchemaVocabulary<LyricAssemblerNs, LyricAssemblerId>
    kLyricAssemblerVocabulary(&kLyricAssemblerNs, &kLyricAssemblerResources);
};

#endif // LYRIC_SCHEMA_ASSEMBLER_SCHEMA_H
