#ifndef LYRIC_SCHEMA_ASSEMBLER_SCHEMA_H
#define LYRIC_SCHEMA_ASSEMBLER_SCHEMA_H

#include <array>

#include <tempo_schema/schema.h>
#include <tempo_schema/schema_namespace.h>

namespace lyric_schema {

    class LyricAssemblerNs : public tempo_schema::SchemaNs {
    public:
        constexpr LyricAssemblerNs() : tempo_schema::SchemaNs("dev.zuri.ns:lyric_assembler") {};
    };
    constexpr LyricAssemblerNs kLyricAssemblerNs;

    enum class LyricAssemblerId {

        // Assembler classes

        Op,                         // execute op with the given opcode
        Trap,                       // invoke trap with the given trap number
        AllocatorTrap,              // use the given trap as allocator
        Plugin,                     // the plugin used to provide traps invoked by the module
        LoadData,                   // load data onto the data stack
        StoreData,                  // pop the top of the data stack and store value

        // Assembler properties

        OpcodeEnum,                 // enum identifying the op to execute
        TrapName,                   // string identifying the trap in the plugin
        DefinitionSymbolPath,       // symbol path identifying the symbol path of the current definition
        StackOffset,                // int offset for stack manipulation ops

        NUM_IDS,                    // must be last
    };

    constexpr tempo_schema::SchemaClass<LyricAssemblerNs,LyricAssemblerId> kLyricAssemblerOpClass(
        &kLyricAssemblerNs, LyricAssemblerId::Op, "Op");

    constexpr tempo_schema::SchemaClass<LyricAssemblerNs,LyricAssemblerId> kLyricAssemblerTrapClass(
        &kLyricAssemblerNs, LyricAssemblerId::Trap, "Trap");

    constexpr tempo_schema::SchemaClass<LyricAssemblerNs,LyricAssemblerId> kLyricAssemblerAllocatorTrapClass(
        &kLyricAssemblerNs, LyricAssemblerId::AllocatorTrap, "AllocatorTrap");

    constexpr tempo_schema::SchemaClass<LyricAssemblerNs,LyricAssemblerId> kLyricAssemblerPluginClass(
        &kLyricAssemblerNs, LyricAssemblerId::Plugin, "Plugin");

    constexpr tempo_schema::SchemaClass<LyricAssemblerNs,LyricAssemblerId> kLyricAssemblerLoadDataClass(
        &kLyricAssemblerNs, LyricAssemblerId::LoadData, "LoadData");

    constexpr tempo_schema::SchemaClass<LyricAssemblerNs,LyricAssemblerId> kLyricAssemblerStoreDataClass(
        &kLyricAssemblerNs, LyricAssemblerId::StoreData, "StoreData");

    constexpr tempo_schema::SchemaProperty<LyricAssemblerNs,LyricAssemblerId>
        kLyricAssemblerOpcodeEnumProperty(
        &kLyricAssemblerNs, LyricAssemblerId::OpcodeEnum, "OpcodeEnum", tempo_schema::PropertyType::kUInt32);

    constexpr tempo_schema::SchemaProperty<LyricAssemblerNs,LyricAssemblerId>
        kLyricAssemblerTrapNameProperty(
        &kLyricAssemblerNs, LyricAssemblerId::TrapName, "TrapName", tempo_schema::PropertyType::kString);

    constexpr tempo_schema::SchemaProperty<LyricAssemblerNs,LyricAssemblerId>
        kLyricAssemblerDefinitionSymbolPathProperty(
        &kLyricAssemblerNs, LyricAssemblerId::DefinitionSymbolPath, "DefinitionSymbolPath", tempo_schema::PropertyType::kString);

    constexpr tempo_schema::SchemaProperty<LyricAssemblerNs,LyricAssemblerId>
        kLyricAssemblerStackOffsetProperty(
        &kLyricAssemblerNs, LyricAssemblerId::StackOffset, "StackOffset", tempo_schema::PropertyType::kUInt32);

    constexpr std::array<
        const tempo_schema::SchemaResource<LyricAssemblerNs,LyricAssemblerId> *,
        static_cast<std::size_t>(LyricAssemblerId::NUM_IDS)>
    kLyricAssemblerResources = {
        &kLyricAssemblerOpClass,
        &kLyricAssemblerTrapClass,
        &kLyricAssemblerAllocatorTrapClass,
        &kLyricAssemblerPluginClass,
        &kLyricAssemblerLoadDataClass,
        &kLyricAssemblerStoreDataClass,
        &kLyricAssemblerOpcodeEnumProperty,
        &kLyricAssemblerTrapNameProperty,
        &kLyricAssemblerDefinitionSymbolPathProperty,
        &kLyricAssemblerStackOffsetProperty,
    };

    constexpr tempo_schema::SchemaVocabulary<LyricAssemblerNs, LyricAssemblerId>
    kLyricAssemblerVocabulary(&kLyricAssemblerNs, &kLyricAssemblerResources);
};

#endif // LYRIC_SCHEMA_ASSEMBLER_SCHEMA_H
