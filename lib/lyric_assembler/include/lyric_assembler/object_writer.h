#ifndef LYRIC_ASSEMBLER_OBJECT_WRITER_H
#define LYRIC_ASSEMBLER_OBJECT_WRITER_H

#include <lyric_object/lyric_object.h>
#include <lyric_object/object_types.h>

#include "object_state.h"

namespace lyric_assembler {

    struct SymbolDefinition {
        lyric_object::LinkageSection section = lyric_object::LinkageSection::Invalid;
        tu_uint32 index = lyric_object::INVALID_ADDRESS_U32;
    };

    struct SymbolEntry {
        enum class EntryType { Invalid, Descriptor, Link };
        EntryType type = EntryType::Invalid;
        tu_uint32 index = lyric_object::INVALID_ADDRESS_U32;
    };

    class ObjectWriter {
    public:
        explicit ObjectWriter(const ObjectState *state);

        tempo_utils::Status initialize();

        tempo_utils::Status touchAction(const ActionSymbol *actionSymbol);
        tempo_utils::Status touchBinding(const BindingSymbol *bindingSymbol);
        tempo_utils::Status touchCall(const CallSymbol *callSymbol);
        tempo_utils::Status touchClass(const ClassSymbol *classSymbol);
        tempo_utils::Status touchConcept(const ConceptSymbol *conceptSymbol);
        tempo_utils::Status touchEnum(const EnumSymbol *enumSymbol);
        tempo_utils::Status touchExistential(const ExistentialSymbol *existentialSymbol);
        tempo_utils::Status touchField(const FieldSymbol *fieldSymbol);
        tempo_utils::Status touchImpl(const ImplHandle *implHandle);
        tempo_utils::Status touchInstance(const InstanceSymbol *instanceSymbol);
        tempo_utils::Status touchLiteral(const LiteralHandle *literalHandle);
        tempo_utils::Status touchNamespace(const NamespaceSymbol *namespaceSymbol);
        tempo_utils::Status touchStatic(const StaticSymbol *staticSymbol);
        tempo_utils::Status touchStruct(const StructSymbol *structSymbol);
        tempo_utils::Status touchTemplate(const TemplateHandle *templateHandle);
        tempo_utils::Status touchType(const TypeHandle *typeHandle);

        tempo_utils::Status touchConstructor(const lyric_common::SymbolUrl &ctor);
        tempo_utils::Status touchInitializer(const lyric_common::SymbolUrl &init);
        tempo_utils::Status touchMember(const DataReference &member);
        tempo_utils::Status touchMethod(const BoundMethod &method);
        tempo_utils::Status touchAction(const ActionMethod &action);
        tempo_utils::Status touchExtension(const ExtensionMethod &extension);
        tempo_utils::Status touchType(const lyric_common::TypeDef &typeDef);

        tempo_utils::Status insertSymbol(
            const lyric_common::SymbolUrl &symbolUrl,
            const AbstractSymbol *symbol,
            bool &alreadyInserted);

        tempo_utils::Status insertImpl(
            const ImplRef &implRef,
            const ImplHandle *implHandle,
            bool &alreadyInserted);

        tempo_utils::Status insertType(
            const lyric_common::TypeDef &typeDef,
            const TypeHandle *typeHandle,
            bool &alreadyInserted);

        tempo_utils::Status insertTemplate(
            const lyric_common::SymbolUrl &templateUrl,
            const TemplateHandle *templateHandle,
            bool &alreadyInserted);

        tempo_utils::Status insertImport(const lyric_common::ModuleLocation &location);

        tempo_utils::Result<SymbolDefinition> getSymbolDefinition(
            const lyric_common::SymbolUrl &symbolUrl) const;
        Option<SymbolDefinition> getSymbolDefinitionOption(const lyric_common::SymbolUrl &symbolUrl) const;
        tempo_utils::Result<lyric_object::LinkageSection> getSymbolSection(const lyric_common::SymbolUrl &symbolUrl) const;
        tempo_utils::Result<tu_uint32> getSymbolAddress(const lyric_common::SymbolUrl &symbolUrl) const;
        tempo_utils::Result<tu_uint32> getLinkAddress(const lyric_common::SymbolUrl &symbolUrl) const;
        tempo_utils::Result<tu_uint32> getSectionAddress(
            const lyric_common::SymbolUrl &symbolUrl,
            lyric_object::LinkageSection section) const;
        tempo_utils::Result<tu_uint32> getLiteralAddress(const LiteralHandle *literalHandle) const;
        tempo_utils::Result<tu_uint32> getImplOffset(const ImplRef &implRef) const;
        tempo_utils::Result<tu_uint32> getTemplateOffset(const lyric_common::SymbolUrl &templateUrl) const;
        tempo_utils::Result<tu_uint32> getTypeOffset(const lyric_common::TypeDef &typeDef) const;
        tempo_utils::Result<tu_uint32> getTrapNumber(
            const lyric_common::ModuleLocation &pluginLocation,
            std::string_view trapName) const;
        tempo_utils::Result<tu_uint32> getTrapNumber(std::string_view trapName) const;

        std::vector<SymbolDefinition>::const_iterator symbolDefinitionsBegin() const;
        std::vector<SymbolDefinition>::const_iterator symbolDefinitionsEnd() const;

        absl::flat_hash_map<lyric_common::SymbolUrl,SymbolEntry>::const_iterator symbolEntriesBegin() const;
        absl::flat_hash_map<lyric_common::SymbolUrl,SymbolEntry>::const_iterator symbolEntriesEnd() const;

        tempo_utils::Result<lyric_object::LyricObject> toObject() const;

    private:
        const ObjectState *m_state;
        bool m_includeUnusedPrivateSymbols;
        bool m_includeUnusedImports;
        std::vector<const ActionSymbol *> m_actions;
        std::vector<const BindingSymbol *> m_bindings;
        std::vector<const CallSymbol *> m_calls;
        std::vector<const ClassSymbol *> m_classes;
        std::vector<const ConceptSymbol *> m_concepts;
        std::vector<const FieldSymbol *> m_fields;
        std::vector<const EnumSymbol *> m_enums;
        std::vector<const ExistentialSymbol *> m_existentials;
        std::vector<const ImplHandle *> m_impls;
        std::vector<const InstanceSymbol *> m_instances;
        std::vector<const LiteralHandle *> m_literals;
        std::vector<const NamespaceSymbol *> m_namespaces;
        std::vector<const StaticSymbol *> m_statics;
        std::vector<const StructSymbol *> m_structs;
        std::vector<const TemplateHandle *> m_templates;
        std::vector<const TypeHandle *> m_types;
        std::vector<const UndeclaredSymbol *> m_undecls;

        std::vector<const ImportHandle *> m_imports;
        std::vector<SymbolDefinition> m_symbols;
        std::vector<RequestedLink> m_links;

        absl::flat_hash_map<lyric_common::SymbolUrl,SymbolEntry> m_symbolEntries;

        absl::flat_hash_map<const LiteralHandle *,tu_uint32> m_literalOffsets;
        absl::flat_hash_map<ImplRef,tu_uint32> m_implOffsets;
        absl::flat_hash_map<lyric_common::ModuleLocation,tu_uint32> m_importOffsets;
        absl::flat_hash_map<lyric_common::SymbolUrl,tu_uint32> m_templateOffsets;
        absl::flat_hash_map<lyric_common::TypeDef,tu_uint32> m_typeOffsets;
    };
}

#endif // LYRIC_ASSEMBLER_OBJECT_WRITER_H
