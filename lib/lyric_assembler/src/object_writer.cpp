
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/internal/write_symbols.h>
#include <lyric_assembler/internal/writer_utils.h>
#include <lyric_assembler/internal/write_actions.h>
#include <lyric_assembler/internal/write_bindings.h>
#include <lyric_assembler/internal/write_calls.h>
#include <lyric_assembler/internal/write_classes.h>
#include <lyric_assembler/internal/write_concepts.h>
#include <lyric_assembler/internal/write_enums.h>
#include <lyric_assembler/internal/write_existentials.h>
#include <lyric_assembler/internal/write_fields.h>
#include <lyric_assembler/internal/write_impls.h>
#include <lyric_assembler/internal/write_imports.h>
#include <lyric_assembler/internal/write_instances.h>
#include <lyric_assembler/internal/write_links.h>
#include <lyric_assembler/internal/write_literals.h>
#include <lyric_assembler/internal/write_namespaces.h>
#include <lyric_assembler/internal/write_statics.h>
#include <lyric_assembler/internal/write_structs.h>
#include <lyric_assembler/internal/write_templates.h>
#include <lyric_assembler/internal/write_types.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/object_writer.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_assembler/linkage_symbol.h>
#include <tempo_utils/memory_bytes.h>

#include "lyric_assembler/object_plugin.h"

lyric_assembler::ObjectWriter::ObjectWriter(const ObjectState *state)
    : m_state(state),
      m_includeUnusedPrivateSymbols(true),
      m_includeUnusedImports(true)
{
    TU_ASSERT (m_state != nullptr);
}

tempo_utils::Status
lyric_assembler::ObjectWriter::initialize()
{
    auto *symbolCache = m_state->symbolCache();

    // walk the entry call
    lyric_common::SymbolUrl entryUrl(m_state->getLocation(), lyric_common::SymbolPath({"$entry"}));
    auto *entrySymbol = symbolCache->getSymbolOrNull(entryUrl);
    if (entrySymbol != nullptr) {
        if (entrySymbol->getSymbolType() != SymbolType::CALL)
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid symbol $entry");
        TU_RETURN_IF_NOT_OK (touchCall(cast_symbol_to_call(entrySymbol)));
    }

    // walk the global namespace
    lyric_common::SymbolUrl globalUrl(m_state->getLocation(), lyric_common::SymbolPath({"$global"}));
    auto *globalSymbol = symbolCache->getSymbolOrNull(globalUrl);
    if (globalSymbol != nullptr) {
        if (globalSymbol->getSymbolType() != SymbolType::NAMESPACE)
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid symbol $global");
        TU_RETURN_IF_NOT_OK (touchNamespace(cast_symbol_to_namespace(globalSymbol)));
    }

    // touch linkages
    for (auto iterator = m_state->linkagesBegin(); iterator != m_state->linkagesEnd(); iterator++) {
        auto &linkageSymbol = *iterator;
        auto linkageUrl = linkageSymbol->getSymbolUrl();

        auto section = internal::linkage_to_descriptor(linkageSymbol->getLinkage());
        if (section == lyo1::DescriptorSection::Invalid)
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "invalid linkage symbol {}: could not parse linkage section", linkageUrl.toString());

        SymbolEntry symbolEntry;
        symbolEntry.type = SymbolEntry::EntryType::Descriptor;
        symbolEntry.index = static_cast<tu_uint32>(m_symbols.size());
        SymbolDefinition symbolDefinition;
        symbolDefinition.section = linkageSymbol->getLinkage();
        symbolDefinition.index = lyric_object::INVALID_ADDRESS_U32;
        m_symbols.push_back(symbolDefinition);
        m_symbolEntries[linkageUrl] = symbolEntry;
    }

    // if we are including all symbols then insert all imports
    if (m_includeUnusedImports) {
        auto *importCache = m_state->importCache();
        for (auto it = importCache->importsBegin(); it != importCache->importsEnd(); it++) {
            TU_RETURN_IF_NOT_OK (insertImport(it->first));
        }
    }

    return {};
}

lyric_common::ModuleLocation
lyric_assembler::ObjectWriter::getOrigin() const
{
    return m_state->getOrigin();
}

tempo_utils::Result<tu_uint32>
lyric_assembler::ObjectWriter::getLiteralAddress(const LiteralHandle *literalHandle) const
{
    auto entry = m_literalOffsets.find(literalHandle);
    if (entry == m_literalOffsets.cend())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "missing literal");
    return entry->second;
}

tempo_utils::Result<lyric_assembler::SymbolDefinition>
lyric_assembler::ObjectWriter::getSymbolDefinition(
    const lyric_common::SymbolUrl &symbolUrl) const
{
    auto entry = m_symbolEntries.find(symbolUrl);
    if (entry == m_symbolEntries.cend())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "missing symbol {}", symbolUrl.toString());

    auto &symbolEntry = entry->second;
    switch (symbolEntry.type) {
        case SymbolEntry::EntryType::Descriptor: {
            if (m_symbols.size() <= symbolEntry.index)
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kAssemblerInvariant, "invalid symbol {}", symbolUrl.toString());
            return m_symbols.at(symbolEntry.index);
        }
        default:
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "no definition available for imported symbol {}", symbolUrl.toString());
    }
}

Option<lyric_assembler::SymbolDefinition>
lyric_assembler::ObjectWriter::getSymbolDefinitionOption(
    const lyric_common::SymbolUrl &symbolUrl) const
{
    auto entry = m_symbolEntries.find(symbolUrl);
    if (entry == m_symbolEntries.cend())
        return {};

    auto &symbolEntry = entry->second;
    switch (symbolEntry.type) {
        case SymbolEntry::EntryType::Descriptor: {
            if (m_symbols.size() <= symbolEntry.index)
                return {};
            return Option(m_symbols.at(symbolEntry.index));
        }
        default:
            return {};
    }
}

tempo_utils::Result<lyric_object::LinkageSection>
lyric_assembler::ObjectWriter::getSymbolSection(const lyric_common::SymbolUrl &symbolUrl) const
{
    auto entry = m_symbolEntries.find(symbolUrl);
    if (entry == m_symbolEntries.cend())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "missing symbol {}", symbolUrl.toString());

    auto &symbolEntry = entry->second;
    switch (symbolEntry.type) {
        case SymbolEntry::EntryType::Descriptor: {
            if (m_symbols.size() <= symbolEntry.index)
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kAssemblerInvariant, "invalid symbol {}", symbolUrl.toString());
            auto symbolDefinition = m_symbols.at(symbolEntry.index);
            return symbolDefinition.section;
        }
        case SymbolEntry::EntryType::Link: {
            if (m_links.size() <= symbolEntry.index)
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kAssemblerInvariant, "invalid link {}", symbolUrl.toString());
            auto requestedLink = m_links.at(symbolEntry.index);
            return requestedLink.linkType;
        }
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid symbol {}", symbolUrl.toString());
    }
}

tempo_utils::Result<tu_uint32>
lyric_assembler::ObjectWriter::getSymbolAddress(const lyric_common::SymbolUrl &symbolUrl) const
{
    auto entry = m_symbolEntries.find(symbolUrl);
    if (entry == m_symbolEntries.cend())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "missing symbol {}", symbolUrl.toString());

    auto &symbolEntry = entry->second;
    switch (symbolEntry.type) {
        case SymbolEntry::EntryType::Descriptor: {
            if (m_symbols.size() <= symbolEntry.index)
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kAssemblerInvariant, "invalid symbol {}", symbolUrl.toString());
            return lyric_object::GET_DESCRIPTOR_ADDRESS(symbolEntry.index);
        }
        case SymbolEntry::EntryType::Link: {
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "missing symbol {}", symbolUrl.toString());
        }
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid symbol {}", symbolUrl.toString());
    }
}

tempo_utils::Result<tu_uint32>
lyric_assembler::ObjectWriter::getLinkAddress(const lyric_common::SymbolUrl &symbolUrl) const
{
    auto entry = m_symbolEntries.find(symbolUrl);
    if (entry == m_symbolEntries.cend())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "missing link {}", symbolUrl.toString());

    auto &symbolEntry = entry->second;
    switch (symbolEntry.type) {
        case SymbolEntry::EntryType::Link: {
            if (m_links.size() <= symbolEntry.index)
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kAssemblerInvariant, "invalid link {}", symbolUrl.toString());
            return lyric_object::GET_LINK_ADDRESS(symbolEntry.index);
        }
        case SymbolEntry::EntryType::Descriptor: {
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "missing link {}", symbolUrl.toString());
        }
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid link {}", symbolUrl.toString());
    }
}

tempo_utils::Result<tu_uint32>
lyric_assembler::ObjectWriter::getSectionAddress(
    const lyric_common::SymbolUrl &symbolUrl,
    lyric_object::LinkageSection section) const
{
    auto entry = m_symbolEntries.find(symbolUrl);
    if (entry == m_symbolEntries.cend())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "missing symbol {}", symbolUrl.toString());

    auto &symbolEntry = entry->second;
    switch (symbolEntry.type) {
        case SymbolEntry::EntryType::Descriptor: {
            if (m_symbols.size() <= symbolEntry.index)
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kAssemblerInvariant, "invalid symbol {}", symbolUrl.toString());
            auto symbolDefinition = m_symbols.at(symbolEntry.index);
            if (symbolDefinition.section != section)
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kAssemblerInvariant, "invalid symbol {}", symbolUrl.toString());
            return lyric_object::GET_DESCRIPTOR_ADDRESS(symbolDefinition.index);
        }
        case SymbolEntry::EntryType::Link: {
            if (m_links.size() <= symbolEntry.index)
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kAssemblerInvariant, "invalid link {}", symbolUrl.toString());
            auto requestedLink = m_links.at(symbolEntry.index);
            if (requestedLink.linkType != section)
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kAssemblerInvariant, "invalid link {}", symbolUrl.toString());
            return lyric_object::GET_LINK_ADDRESS(symbolEntry.index);
        }
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid symbol {}", symbolUrl.toString());
    }
}

tempo_utils::Result<tu_uint32>
lyric_assembler::ObjectWriter::getImplOffset(const ImplRef &implRef) const
{
    auto entry = m_implOffsets.find(implRef);
    if (entry == m_implOffsets.cend())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "missing impl");
    return entry->second;
}

tempo_utils::Result<tu_uint32>
lyric_assembler::ObjectWriter::getTemplateOffset(const lyric_common::SymbolUrl &templateUrl) const
{
    auto entry = m_templateOffsets.find(templateUrl);
    if (entry == m_templateOffsets.cend())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "missing template");
    return entry->second;
}

tempo_utils::Result<tu_uint32>
lyric_assembler::ObjectWriter::getTypeOffset(const lyric_common::TypeDef &typeDef) const
{
    auto entry = m_typeOffsets.find(typeDef);
    if (entry == m_typeOffsets.cend())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "missing type");
    return entry->second;
}

tempo_utils::Result<tu_uint32>
lyric_assembler::ObjectWriter::getTrapNumber(
    const lyric_common::ModuleLocation &pluginLocation,
    std::string_view trapName) const
{
    auto *plugin = m_state->objectPlugin();
    if (plugin == nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid object plugin");
    if (pluginLocation != plugin->getLocation())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "unexpected plugin location {} for trap {}",
            pluginLocation.toString(), trapName);
    auto trapNumber = plugin->lookupTrap(trapName);
    if (trapNumber == lyric_object::INVALID_ADDRESS_U32)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid trap {}", trapName);
    return trapNumber;
}

tempo_utils::Result<tu_uint32>
lyric_assembler::ObjectWriter::getTrapNumber(std::string_view trapName) const
{
    auto *plugin = m_state->objectPlugin();
    if (plugin == nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid object plugin");
    auto trapNumber = plugin->lookupTrap(trapName);
    if (trapNumber == lyric_object::INVALID_ADDRESS_U32)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid trap {}", trapName);
    return trapNumber;
}

std::vector<lyric_assembler::SymbolDefinition>::const_iterator
lyric_assembler::ObjectWriter::symbolDefinitionsBegin() const
{
    return m_symbols.cbegin();
}

std::vector<lyric_assembler::SymbolDefinition>::const_iterator
lyric_assembler::ObjectWriter::symbolDefinitionsEnd() const
{
    return m_symbols.cend();
}

absl::flat_hash_map<lyric_common::SymbolUrl,lyric_assembler::SymbolEntry>::const_iterator
lyric_assembler::ObjectWriter::symbolEntriesBegin() const
{
    return m_symbolEntries.cbegin();
}

absl::flat_hash_map<lyric_common::SymbolUrl,lyric_assembler::SymbolEntry>::const_iterator
lyric_assembler::ObjectWriter::symbolEntriesEnd() const
{
    return m_symbolEntries.cend();
}

tempo_utils::Status
lyric_assembler::ObjectWriter::insertImport(const lyric_common::ModuleLocation &location)
{
    if (m_importOffsets.contains(location))
        return {};
    auto *importCache = m_state->importCache();
    auto *importHandle = importCache->getImport(location);
    TU_ASSERT (importHandle != nullptr);
    tu_uint32 importOffset = m_imports.size();
    m_imports.push_back(importHandle);
    m_importOffsets[location] = importOffset;
    return {};
}

template<class SymbolType>
static tempo_utils::Status
insert_symbol(
    const SymbolType *symbol,
    std::vector<const SymbolType *> &sectionVector,
    std::vector<lyric_assembler::SymbolDefinition> &symbolsVector,
    std::vector<lyric_assembler::RequestedLink> &linksVector,
    absl::flat_hash_map<lyric_common::SymbolUrl,lyric_assembler::SymbolEntry> &symbolEntries,
    lyric_assembler::ObjectWriter *writer)
{
    auto symbolUrl = symbol->getSymbolUrl();
    auto section = symbol->getLinkage();

    lyric_assembler::SymbolEntry symbolEntry;
    if (symbol->isImported()) {
        TU_RETURN_IF_NOT_OK (writer->insertImport(symbolUrl.getModuleLocation()));
        symbolEntry.type = lyric_assembler::SymbolEntry::EntryType::Link;
        symbolEntry.index = linksVector.size();
        lyric_assembler::RequestedLink requestedLink;
        requestedLink.linkType = section;
        requestedLink.linkUrl = symbolUrl;
        linksVector.push_back(std::move(requestedLink));
    } else {
        symbolEntry.type = lyric_assembler::SymbolEntry::EntryType::Descriptor;
        symbolEntry.index = symbolsVector.size();
        lyric_assembler::SymbolDefinition symbolDefinition;
        symbolDefinition.section = section;
        symbolDefinition.index = sectionVector.size();
        sectionVector.push_back(symbol);
        symbolsVector.push_back(symbolDefinition);
    }
    symbolEntries[symbolUrl] = symbolEntry;
    return {};
}

tempo_utils::Status
lyric_assembler::ObjectWriter::insertSymbol(
    const lyric_common::SymbolUrl &symbolUrl,
    const AbstractSymbol *symbol,
    bool &alreadyInserted)
{
    if (m_symbolEntries.contains(symbolUrl)) {
        alreadyInserted = true;
        return {};
    }

    switch (symbol->getSymbolType()) {

        case SymbolType::ACTION: {
            TU_RETURN_IF_NOT_OK (insert_symbol(cast_symbol_to_action(symbol),
                m_actions, m_symbols, m_links, m_symbolEntries, this));
            break;
        }

        case SymbolType::BINDING: {
            TU_RETURN_IF_NOT_OK (insert_symbol(cast_symbol_to_binding(symbol),
                m_bindings, m_symbols, m_links, m_symbolEntries, this));
            break;
        }

        case SymbolType::CALL: {
            TU_RETURN_IF_NOT_OK (insert_symbol(cast_symbol_to_call(symbol),
                m_calls, m_symbols, m_links, m_symbolEntries, this));
            break;
        }

        case SymbolType::CLASS: {
            TU_RETURN_IF_NOT_OK (insert_symbol(cast_symbol_to_class(symbol),
                m_classes, m_symbols, m_links, m_symbolEntries, this));
            break;
        }

        case SymbolType::CONCEPT: {
            TU_RETURN_IF_NOT_OK (insert_symbol(cast_symbol_to_concept(symbol),
                m_concepts, m_symbols, m_links, m_symbolEntries, this));
            break;
        }

        case SymbolType::ENUM: {
            TU_RETURN_IF_NOT_OK (insert_symbol(cast_symbol_to_enum(symbol),
                m_enums, m_symbols, m_links, m_symbolEntries, this));
            break;
        }

        case SymbolType::EXISTENTIAL: {
            TU_RETURN_IF_NOT_OK (insert_symbol(cast_symbol_to_existential(symbol),
                m_existentials, m_symbols, m_links, m_symbolEntries, this));
            break;
        }

        case SymbolType::FIELD: {
            TU_RETURN_IF_NOT_OK (insert_symbol(cast_symbol_to_field(symbol),
                m_fields, m_symbols, m_links, m_symbolEntries, this));
            break;
        }

        case SymbolType::INSTANCE: {
            TU_RETURN_IF_NOT_OK (insert_symbol(cast_symbol_to_instance(symbol),
                m_instances, m_symbols, m_links, m_symbolEntries, this));
            break;
        }

        case SymbolType::NAMESPACE: {
            TU_RETURN_IF_NOT_OK (insert_symbol(cast_symbol_to_namespace(symbol),
                m_namespaces, m_symbols, m_links, m_symbolEntries, this));
            break;
        }

        case SymbolType::STATIC: {
            TU_RETURN_IF_NOT_OK (insert_symbol(cast_symbol_to_static(symbol),
                m_statics, m_symbols, m_links, m_symbolEntries, this));
            break;
        }

        case SymbolType::STRUCT: {
            TU_RETURN_IF_NOT_OK (insert_symbol(cast_symbol_to_struct(symbol),
                m_structs, m_symbols, m_links, m_symbolEntries, this));
            break;
        }

        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "cannot track symbol");
    }

    alreadyInserted = false;
    return {};
}

tempo_utils::Status
lyric_assembler::ObjectWriter::insertImpl(
    const ImplRef &implRef,
    const ImplHandle *implHandle,
    bool &alreadyInserted)
{
    if (m_implOffsets.contains(implRef)) {
        alreadyInserted = true;
        return {};
    }

    tu_uint32 offset = m_impls.size();
    m_impls.push_back(implHandle);
    m_implOffsets[implRef] = offset;

    alreadyInserted = false;
    return {};
}

tempo_utils::Status
lyric_assembler::ObjectWriter::insertType(
    const lyric_common::TypeDef &typeDef,
    const TypeHandle *typeHandle,
    bool &alreadyInserted)
{
    if (m_typeOffsets.contains(typeDef)) {
        alreadyInserted = true;
        return {};
    }

    tu_uint32 offset = m_types.size();
    m_types.push_back(typeHandle);
    m_typeOffsets[typeDef] = offset;

    alreadyInserted = false;
    return {};
}

tempo_utils::Status
lyric_assembler::ObjectWriter::insertTemplate(
    const lyric_common::SymbolUrl &templateUrl,
    const TemplateHandle *templateHandle,
    bool &alreadyInserted)
{
    if (m_templateOffsets.contains(templateUrl)) {
        alreadyInserted = true;
        return {};
    }

    tu_uint32 offset = m_templates.size();
    m_templates.push_back(templateHandle);
    m_templateOffsets[templateUrl] = offset;

    alreadyInserted = false;
    return {};
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchAction(const ActionSymbol *actionSymbol)
{
    return internal::touch_action(actionSymbol, m_state, *this);
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchBinding(const BindingSymbol *bindingSymbol)
{
    return internal::touch_binding(bindingSymbol, m_state, *this);
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchCall(const CallSymbol *callSymbol)
{
    return internal::touch_call(callSymbol, m_state, *this);
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchClass(const ClassSymbol *classSymbol)
{
    return internal::touch_class(classSymbol, m_state, *this);
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchConcept(const ConceptSymbol *conceptSymbol)
{
    return internal::touch_concept(conceptSymbol, m_state, *this);
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchEnum(const EnumSymbol *enumSymbol)
{
    return internal::touch_enum(enumSymbol, m_state, *this);
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchExistential(const ExistentialSymbol *existentialSymbol)
{
    return internal::touch_existential(existentialSymbol, m_state, *this);
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchField(const FieldSymbol *fieldSymbol)
{
    return internal::touch_field(fieldSymbol, m_state, *this);
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchImpl(const ImplHandle *implHandle)
{
    return internal::touch_impl(implHandle, m_state, *this);
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchInstance(const InstanceSymbol *instanceSymbol)
{
    return internal::touch_instance(instanceSymbol, m_state, *this);
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchLiteral(const LiteralHandle *literalHandle)
{
    if (!m_literalOffsets.contains(literalHandle)) {
        tu_uint32 offset = m_literals.size();
        m_literals.push_back(literalHandle);
        m_literalOffsets[literalHandle] = offset;
    }
    return {};
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchNamespace(const NamespaceSymbol *namespaceSymbol)
{
    return internal::touch_namespace(namespaceSymbol, m_state, *this, m_includeUnusedPrivateSymbols);
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchStatic(const StaticSymbol *staticSymbol)
{
    return internal::touch_static(staticSymbol, m_state, *this);
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchStruct(const StructSymbol *structSymbol)
{
    return internal::touch_struct(structSymbol, m_state, *this);
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchTemplate(const TemplateHandle *templateHandle)
{
    return internal::touch_template(templateHandle, m_state, *this);
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchType(const TypeHandle *typeHandle)
{
    return internal::touch_type(typeHandle, m_state, *this);
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchConstructor(const lyric_common::SymbolUrl &ctor)
{
    auto *symbolCache = m_state->symbolCache();
    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(ctor));
    if (symbol->getSymbolType() != SymbolType::CALL)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid symbol for constructor");
    auto *ctorSymbol = cast_symbol_to_call(symbol);
    if (!ctorSymbol->isCtor())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid symbol for constructor");
    TU_RETURN_IF_NOT_OK (touchCall(ctorSymbol));
    auto receiver = ctorSymbol->getReceiverUrl();
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(receiver));
    switch (symbol->getSymbolType()) {
        case SymbolType::CLASS:
            return touchClass(cast_symbol_to_class(symbol));
        case SymbolType::ENUM:
            return touchEnum(cast_symbol_to_enum(symbol));
        case SymbolType::INSTANCE:
            return touchInstance(cast_symbol_to_instance(symbol));
        case SymbolType::STRUCT:
            return touchStruct(cast_symbol_to_struct(symbol));
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid receiver for constructor");
    }
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchInitializer(const lyric_common::SymbolUrl &init)
{
    auto *symbolCache = m_state->symbolCache();
    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(init));
    if (symbol->getSymbolType() != SymbolType::CALL)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid symbol for initializer");
    auto *callSymbol = cast_symbol_to_call(symbol);
    if (callSymbol->isBound())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid symbol for initializer");
    return touchCall(callSymbol);
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchMember(const DataReference &member)
{
    auto *symbolCache = m_state->symbolCache();
    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(member.symbolUrl));
    if (symbol->getSymbolType() != SymbolType::FIELD)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid symbol for field");
    auto *fieldSymbol = cast_symbol_to_field(symbol);
    return touchField(fieldSymbol);
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchMethod(const BoundMethod &method)
{
    auto *symbolCache = m_state->symbolCache();
    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(method.methodCall));
    if (symbol->getSymbolType() != SymbolType::CALL)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid symbol for method");
    auto *callSymbol = cast_symbol_to_call(symbol);
    if (!callSymbol->isBound())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid symbol for method");
    return touchCall(callSymbol);
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchAction(const ActionMethod &action)
{
    auto *symbolCache = m_state->symbolCache();
    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(action.methodAction));
    if (symbol->getSymbolType() != SymbolType::ACTION)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid symbol for action");
    auto *actionSymbol = cast_symbol_to_action(symbol);
    return touchAction(actionSymbol);
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchExtension(const ExtensionMethod &extension)
{
    auto *symbolCache = m_state->symbolCache();
    AbstractSymbol *symbol;

    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(extension.methodAction));
    if (symbol->getSymbolType() != SymbolType::ACTION)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid symbol for action");
    auto *actionSymbol = cast_symbol_to_action(symbol);
    TU_RETURN_IF_NOT_OK (touchAction(actionSymbol));

    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(extension.methodCall));
    if (symbol->getSymbolType() != SymbolType::CALL)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid symbol for extension");
    auto *callSymbol = cast_symbol_to_call(symbol);
    if (!callSymbol->isBound())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid symbol for extension");
    TU_RETURN_IF_NOT_OK (touchCall(callSymbol));

    return {};
}

tempo_utils::Status
lyric_assembler::ObjectWriter::touchType(const lyric_common::TypeDef &typeDef)
{
    auto *typeCache = m_state->typeCache();
    TypeHandle *typeHandle;
    TU_ASSIGN_OR_RETURN (typeHandle, typeCache->getOrMakeType(typeDef));
    return internal::touch_type(typeHandle, m_state, *this);
}

tempo_utils::Result<lyric_object::LyricObject>
lyric_assembler::ObjectWriter::toObject() const
{
    flatbuffers::FlatBufferBuilder buffer;

    std::vector<flatbuffers::Offset<lyo1::PluginDescriptor>> plugins_vector;
    std::vector<uint8_t> bytecode;

    lyo1::ObjectVersion version = lyo1::ObjectVersion::Version1;

    // serialize array of action descriptors
    internal::ActionsOffset actionsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_actions(m_actions, *this, buffer, actionsOffset));

    // serialize array of binding descriptors
    internal::BindingsOffset bindingsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_bindings(m_bindings, *this, buffer, bindingsOffset));

    // serialize array of call descriptors
    internal::CallsOffset callsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_calls(m_calls, *this, buffer, callsOffset, bytecode));

    // serialize array of class descriptors
    internal::ClassesOffset classesOffset;
    TU_RETURN_IF_NOT_OK (internal::write_classes(m_classes, *this, buffer, classesOffset));

    // serialize array of concept descriptors
    internal::ConceptsOffset conceptsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_concepts(m_concepts, *this, buffer, conceptsOffset));

    // serialize array of enum descriptors
    internal::EnumsOffset enumsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_enums(m_enums, *this, buffer, enumsOffset));

    // serialize array of field descriptors
    internal::FieldsOffset fieldsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_fields(m_fields, *this, buffer, fieldsOffset));

    // serialize array of impl descriptors
    internal::ImplsOffset implsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_impls(m_impls, *this, buffer, implsOffset));

    // serialize array of import descriptors
    internal::ImportsOffset importsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_imports(m_imports, *this, buffer, importsOffset));

    // serialize array of instance descriptors
    internal::InstancesOffset instancesOffset;
    TU_RETURN_IF_NOT_OK (internal::write_instances(m_instances, *this, buffer, instancesOffset));

    // serialize arrays of link descriptors
    internal::LinksOffset linksOffset;
    TU_RETURN_IF_NOT_OK (internal::write_links(m_links, *this, m_importOffsets, buffer, linksOffset));

    // serialize array of literal descriptors
    internal::LiteralsOffset literalsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_literals(m_literals, *this, buffer, literalsOffset));

    // serialize array of namespace descriptors
    internal::NamespacesOffset namespacesOffset;
    TU_RETURN_IF_NOT_OK (internal::write_namespaces(
        m_namespaces, *this, m_state->getLocation(), buffer, namespacesOffset));

    // serialize array of static descriptors
    internal::StaticsOffset staticsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_statics(m_statics, *this, buffer, staticsOffset));

    // serialize array of struct descriptors
    internal::StructsOffset structsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_structs(m_structs, *this, buffer, structsOffset));

    // serialize array of template descriptors
    internal::TemplatesOffset templatesOffset;
    TU_RETURN_IF_NOT_OK (internal::write_templates(m_templates, *this, buffer, templatesOffset));

    // serialize array of type descriptors
    internal::TypesOffset typesOffset;
    TU_RETURN_IF_NOT_OK (internal::write_types(m_types, *this, buffer, typesOffset));

    // serialize array of symbol descriptors
    internal::SymbolsOffset symbolsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_symbols(*this, buffer, symbolsOffset));

    // serialize the symbol table
    internal::SortedSymbolTableOffset sortedSymbolTableOffset;
    TU_RETURN_IF_NOT_OK (internal::write_sorted_symbol_table(*this, buffer, sortedSymbolTableOffset));

    auto options = m_state->getOptions();

    // serialize array of plugin descriptors
    flatbuffers::Offset<lyo1::PluginDescriptor> optionalPluginOffset = 0;
    auto *plugin = m_state->objectPlugin();
    if (plugin != nullptr) {
        auto pluginLocation = m_state->getPluginLocation();
        auto plugin_location = buffer.CreateSharedString(pluginLocation.toString());
        optionalPluginOffset = lyo1::CreatePluginDescriptor(buffer, plugin_location);
    }

    // serialize vectors
    auto bytecodeOffset = buffer.CreateVector(bytecode);

    // build assembly from buffer
    lyo1::ObjectBuilder objectBuilder(buffer);

    // set abi and assembly version
    objectBuilder.add_abi(version);
    objectBuilder.add_version_major(options->majorVersion);
    objectBuilder.add_version_minor(options->minorVersion);
    objectBuilder.add_version_patch(options->patchVersion);

    objectBuilder.add_actions(actionsOffset);
    objectBuilder.add_bindings(bindingsOffset);
    objectBuilder.add_calls(callsOffset);
    objectBuilder.add_classes(classesOffset);
    objectBuilder.add_concepts(conceptsOffset);
    objectBuilder.add_enums(enumsOffset);
    objectBuilder.add_fields(fieldsOffset);
    objectBuilder.add_impls(implsOffset);
    objectBuilder.add_imports(importsOffset);
    objectBuilder.add_instances(instancesOffset);
    objectBuilder.add_links(linksOffset);
    objectBuilder.add_literals(literalsOffset);
    objectBuilder.add_namespaces(namespacesOffset);
    objectBuilder.add_statics(staticsOffset);
    objectBuilder.add_structs(structsOffset);
    objectBuilder.add_symbols(symbolsOffset);
    objectBuilder.add_templates(templatesOffset);
    objectBuilder.add_types(typesOffset);
    objectBuilder.add_plugin(optionalPluginOffset);
    objectBuilder.add_symbol_table_type(lyo1::SymbolTable::SortedSymbolTable);
    objectBuilder.add_symbol_table(sortedSymbolTableOffset.Union());

    objectBuilder.add_bytecode(bytecodeOffset);

    // serialize assembly and mark the buffer as finished
    auto object = objectBuilder.Finish();
    buffer.Finish(object, lyo1::ObjectIdentifier());

    // copy the flatbuffer into our own byte array and instantiate object
    auto bytes = tempo_utils::MemoryBytes::copy(buffer.GetBufferSpan());
    return lyric_object::LyricObject(bytes);
}