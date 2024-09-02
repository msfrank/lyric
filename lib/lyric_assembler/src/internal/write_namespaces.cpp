
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

inline bool
is_in_scope(lyric_object::AccessType access, bool includeUnusedPrivateSymbols)
{
    if (includeUnusedPrivateSymbols)
        return true;
    switch (access) {
        case lyric_object::AccessType::Public:
        case lyric_object::AccessType::Protected:
            return true;
        default:
            return false;
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
    auto *namespaceBlock = namespaceSymbol->namespaceBlock();

    for (auto symbolIterator = namespaceBlock->symbolsBegin();
        symbolIterator != namespaceBlock->symbolsEnd();
        symbolIterator++) {
        const auto &var = symbolIterator->second;
        const auto &symbolUrl = var.symbolUrl;

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
    std::vector<flatbuffers::Offset<lyo1::NamespaceDescriptor>> &namespaces_vector,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    auto index = static_cast<tu_uint32>(namespaces_vector.size());

    auto namespacePathString = namespaceSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fullyQualifiedName = buffer.CreateSharedString(namespacePathString);

    auto supernamespaceIndex = lyric_runtime::INVALID_ADDRESS_U32;
    auto *supernamespaceSymbol = namespaceSymbol->superNamespace();
    if (supernamespaceSymbol != nullptr) {
        TU_ASSIGN_OR_RETURN (supernamespaceIndex,
            writer.getSymbolAddress(supernamespaceSymbol->getSymbolUrl(), lyric_object::LinkageSection::Namespace));
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

    auto *namespaceBlock = namespaceSymbol->namespaceBlock();

    // serialize array of symbols
    std::vector<lyo1::NamespaceBinding> bindings;
    for (auto symbolIterator = namespaceBlock->symbolsBegin();
         symbolIterator != namespaceBlock->symbolsEnd();
         symbolIterator++) {
        const auto &var = symbolIterator->second;
        const auto &symbolUrl = var.symbolUrl;

        // skip symbols which are not in the current module
        if (symbolUrl.isAbsolute()) {
            if (location != symbolUrl.getModuleLocation()) {
                TU_LOG_INFO << "ignoring namespace binding " << symbolIterator->first << " for " << symbolUrl;
                continue;
            }
        }

        // skip symbols which have not been touched
        auto symbolEntryOption = writer.getSymbolEntryOption(symbolUrl);
        if (symbolEntryOption.isEmpty())
            continue;
        const auto &symbolEntry = symbolEntryOption.peekValue();

        lyo1::DescriptorSection binding_type = lyo1::DescriptorSection::Invalid;
        tu_uint32 binding_descriptor;

        switch (symbolEntry.section) {
            case lyric_object::LinkageSection::Call: {
                binding_type = lyo1::DescriptorSection::Call;
                binding_descriptor = symbolEntry.address;
                break;
            }
            case lyric_object::LinkageSection::Class: {
                binding_type = lyo1::DescriptorSection::Class;
                binding_descriptor = symbolEntry.address;
                break;
            }
            case lyric_object::LinkageSection::Concept: {
                binding_type = lyo1::DescriptorSection::Concept;
                binding_descriptor = symbolEntry.address;
                break;
            }
            case lyric_object::LinkageSection::Enum: {
                binding_type = lyo1::DescriptorSection::Enum;
                binding_descriptor = symbolEntry.address;
                break;
            }
            case lyric_object::LinkageSection::Instance: {
                binding_type = lyo1::DescriptorSection::Instance;
                binding_descriptor = symbolEntry.address;
                break;
            }
            case lyric_object::LinkageSection::Namespace: {
                binding_type = lyo1::DescriptorSection::Namespace;
                binding_descriptor = symbolEntry.address;
                break;
            }
            case lyric_object::LinkageSection::Static: {
                binding_type = lyo1::DescriptorSection::Static;
                binding_descriptor = symbolEntry.address;
                break;
            }
            case lyric_object::LinkageSection::Struct: {
                binding_type = lyo1::DescriptorSection::Struct;
                binding_descriptor = symbolEntry.address;
                break;
            }
            default: {
                TU_LOG_VV << "ignoring namespace binding " << symbolIterator->first
                          << " for " << var.symbolUrl.toString();
                binding_type = lyo1::DescriptorSection::Invalid;
                break;
            }
        }

        //
        if (binding_type != lyo1::DescriptorSection::Invalid) {
            bindings.emplace_back(lyo1::NamespaceBinding(binding_type, binding_descriptor));
        }
    }

    // add namespace descriptor
    namespaces_vector.push_back(lyo1::CreateNamespaceDescriptor(buffer, fullyQualifiedName,
        supernamespaceIndex, namespaceFlags, buffer.CreateVectorOfStructs(bindings)));

    // add symbol descriptor
    symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, fullyQualifiedName,
        lyo1::DescriptorSection::Namespace, index));

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_namespaces(
    const std::vector<const NamespaceSymbol *> &namespaces,
    const ObjectWriter &writer,
    const lyric_common::ModuleLocation &location,
    flatbuffers::FlatBufferBuilder &buffer,
    NamespacesOffset &namespacesOffset,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    std::vector<flatbuffers::Offset<lyo1::NamespaceDescriptor>> namespaces_vector;

    for (const auto *namespaceSymbol : namespaces) {
        TU_RETURN_IF_NOT_OK (write_namespace(
            namespaceSymbol, writer, location, buffer, namespaces_vector, symbols_vector));
    }

    // create the namespaces vector
    namespacesOffset = buffer.CreateVector(namespaces_vector);

    return {};
}
