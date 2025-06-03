
#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/internal/write_namespaces.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_assembler/linkage_symbol.h>

inline bool
is_in_scope(lyric_object::AccessType access, bool includeUnusedPrivateSymbols)
{
    switch (access) {
        case lyric_object::AccessType::Public:
        case lyric_object::AccessType::Protected:
            return true;
        default:
            return includeUnusedPrivateSymbols;
    }
}

tempo_utils::Status
lyric_assembler::internal::touch_namespace(
    const NamespaceSymbol *namespaceSymbol,
    const ObjectState *objectState,
    ObjectWriter &writer,
    bool includeUnusedPrivateSymbols)
{
    auto namespaceUrl = namespaceSymbol->getSymbolUrl();

    bool alreadyInserted;
    TU_RETURN_IF_NOT_OK (writer.insertSymbol(namespaceUrl, namespaceSymbol, alreadyInserted));
    if (alreadyInserted)
        return {};

    // if namespace is an imported symbol then we are done
    if (namespaceSymbol->isImported())
        return {};

    auto *symbolCache = objectState->symbolCache();

    for (auto symbolIterator = namespaceSymbol->targetsBegin();
        symbolIterator != namespaceSymbol->targetsEnd();
        symbolIterator++) {
        const auto &symbolUrl = symbolIterator->second;

        AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(symbolUrl));
        switch (symbol->getSymbolType()) {

            // ignore variables
            case SymbolType::ARGUMENT:
            case SymbolType::LEXICAL:
            case SymbolType::LOCAL:
                break;

            case SymbolType::CALL: {
                auto *callSymbol = cast_symbol_to_call(symbol);
                if (is_in_scope(callSymbol->getAccessType(), includeUnusedPrivateSymbols)) {
                    TU_RETURN_IF_NOT_OK (writer.touchCall(callSymbol));
                }
                break;
            }
            case SymbolType::BINDING: {
                auto *bindingSymbol = cast_symbol_to_binding(symbol);
                if (is_in_scope(bindingSymbol->getAccessType(), includeUnusedPrivateSymbols)) {
                    TU_RETURN_IF_NOT_OK (writer.touchBinding(bindingSymbol));
                }
                break;
            }
            case SymbolType::CLASS: {
                auto *classSymbol = cast_symbol_to_class(symbol);
                if (is_in_scope(classSymbol->getAccessType(), includeUnusedPrivateSymbols)) {
                    TU_RETURN_IF_NOT_OK (writer.touchClass(classSymbol));
                }
                break;
            }
            case SymbolType::CONCEPT: {
                auto *conceptSymbol = cast_symbol_to_concept(symbol);
                if (is_in_scope(conceptSymbol->getAccessType(), includeUnusedPrivateSymbols)) {
                    TU_RETURN_IF_NOT_OK (writer.touchConcept(conceptSymbol));
                }
                break;
            }
            case SymbolType::ENUM: {
                auto *enumSymbol = cast_symbol_to_enum(symbol);
                if (is_in_scope(enumSymbol->getAccessType(), includeUnusedPrivateSymbols)) {
                    TU_RETURN_IF_NOT_OK (writer.touchEnum(enumSymbol));
                }
                break;
            }
            case SymbolType::INSTANCE: {
                auto *instanceSymbol = cast_symbol_to_instance(symbol);
                if (is_in_scope(instanceSymbol->getAccessType(), includeUnusedPrivateSymbols)) {
                    TU_RETURN_IF_NOT_OK (writer.touchInstance(instanceSymbol));
                }
                break;
            }
            case SymbolType::NAMESPACE: {
                auto *nsSymbol = cast_symbol_to_namespace(symbol);
                if (is_in_scope(nsSymbol->getAccessType(), includeUnusedPrivateSymbols)) {
                    TU_RETURN_IF_NOT_OK (writer.touchNamespace(nsSymbol));
                }
                break;
            }
            case SymbolType::STATIC: {
                auto *staticSymbol = cast_symbol_to_static(symbol);
                if (is_in_scope(staticSymbol->getAccessType(), includeUnusedPrivateSymbols)) {
                    TU_RETURN_IF_NOT_OK (writer.touchStatic(staticSymbol));
                }
                break;
            }
            case SymbolType::STRUCT: {
                auto *structSymbol = cast_symbol_to_struct(symbol);
                if (is_in_scope(structSymbol->getAccessType(), includeUnusedPrivateSymbols)) {
                    TU_RETURN_IF_NOT_OK (writer.touchStruct(structSymbol));
                }
                break;
            }
            case SymbolType::LINKAGE: {
                break;
            }
            default:
                return AssemblerStatus::forCondition(AssemblerCondition::kInvalidBinding,
                    "invalid binding {} for namespace {}",
                    symbolIterator->first, namespaceSymbol->getSymbolUrl().toString());
        }
    }

    return {};
}

static tempo_utils::Status
write_namespace(
    const lyric_assembler::NamespaceSymbol *namespaceSymbol,
    const lyric_assembler::ObjectWriter &writer,
    const lyric_common::ModuleLocation &location,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::NamespaceDescriptor>> &namespaces_vector)
{
    auto namespacePathString = namespaceSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fullyQualifiedName = buffer.CreateSharedString(namespacePathString);

    auto supernamespaceIndex = lyric_runtime::INVALID_ADDRESS_U32;
    auto *supernamespaceSymbol = namespaceSymbol->superNamespace();
    if (supernamespaceSymbol != nullptr) {
        TU_ASSIGN_OR_RETURN (supernamespaceIndex,
            writer.getSectionAddress(supernamespaceSymbol->getSymbolUrl(), lyric_object::LinkageSection::Namespace));
    }

    lyo1::NamespaceFlags namespaceFlags = lyo1::NamespaceFlags::NONE;
    if (namespaceSymbol->isDeclOnly())
        namespaceFlags |= lyo1::NamespaceFlags::DeclOnly;
    switch (namespaceSymbol->getAccessType()) {
        case lyric_object::AccessType::Public:
            namespaceFlags |= lyo1::NamespaceFlags::GlobalVisibility;
            break;
        case lyric_object::AccessType::Protected:
            namespaceFlags |= lyo1::NamespaceFlags::InheritVisibility;
            break;
        case lyric_object::AccessType::Private:
            break;
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                "invalid namespace access");
    }

    // serialize array of symbols
    std::vector<tu_uint32> bindings;
    for (auto symbolIterator = namespaceSymbol->targetsBegin();
         symbolIterator != namespaceSymbol->targetsEnd();
         symbolIterator++) {
        const auto &symbolUrl = symbolIterator->second;

        // skip symbols which are not in the current module
        if (symbolUrl.isAbsolute()) {
            if (location != symbolUrl.getModuleLocation()) {
                TU_LOG_INFO << "ignoring namespace binding " << symbolIterator->first << " for " << symbolUrl;
                continue;
            }
        }

        tu_uint32 bindingSymbolIndex;
        TU_ASSIGN_OR_RETURN (bindingSymbolIndex, writer.getSymbolAddress(symbolUrl));

        bindings.push_back(bindingSymbolIndex);
    }

    // add namespace descriptor
    namespaces_vector.push_back(lyo1::CreateNamespaceDescriptor(buffer, fullyQualifiedName,
        supernamespaceIndex, namespaceFlags, buffer.CreateVector(bindings)));

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_namespaces(
    const std::vector<const NamespaceSymbol *> &namespaces,
    const ObjectWriter &writer,
    const lyric_common::ModuleLocation &location,
    flatbuffers::FlatBufferBuilder &buffer,
    NamespacesOffset &namespacesOffset)
{
    std::vector<flatbuffers::Offset<lyo1::NamespaceDescriptor>> namespaces_vector;

    for (const auto *namespaceSymbol : namespaces) {
        TU_RETURN_IF_NOT_OK (
            write_namespace(namespaceSymbol, writer, location, buffer, namespaces_vector));
    }

    // create the namespaces vector
    namespacesOffset = buffer.CreateVector(namespaces_vector);

    return {};
}
