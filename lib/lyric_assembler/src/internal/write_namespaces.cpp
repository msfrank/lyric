
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

static tempo_utils::Status
write_namespace(
    lyric_assembler::NamespaceSymbol *namespaceSymbol,
    const lyric_common::AssemblyLocation &location,
    lyric_assembler::TypeCache *typeCache,
    lyric_assembler::SymbolCache *symbolCache,
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
        supernamespaceIndex = supernamespaceSymbol->getAddress().getAddress();
    }

    lyo1::NamespaceFlags namespaceFlags = lyo1::NamespaceFlags::NONE;
    if (!namespaceSymbol->getAddress().isValid())
        namespaceFlags |= lyo1::NamespaceFlags::DeclOnly;

    auto *namespaceBlock = namespaceSymbol->namespaceBlock();

    // serialize array of symbols
    std::vector<lyo1::NamespaceBinding> bindings;
    for (auto symbolIterator = namespaceBlock->symbolsBegin();
         symbolIterator != namespaceBlock->symbolsEnd();
         symbolIterator++) {
        const auto &var = symbolIterator->second;
        lyric_assembler::AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(var.symbolUrl));

        lyo1::DescriptorSection binding_type = lyo1::DescriptorSection::Invalid;
        tu_uint32 binding_descriptor;

        // skip symbols which are not in the current module
        if (var.symbolUrl.isAbsolute()) {
            if (location != symbol->getSymbolUrl().getAssemblyLocation()) {
                TU_LOG_INFO << "ignoring namespace binding " << symbolIterator->first << " for " << symbol->getSymbolUrl();
                continue;
            }
        }

        switch (symbol->getSymbolType()) {
            case lyric_assembler::SymbolType::CALL: {
                auto *callSymbol = cast_symbol_to_call(symbol);
                binding_type = lyo1::DescriptorSection::Call;
                auto address = callSymbol->getAddress();
                TU_ASSERT (address.isValid());
                binding_descriptor = address.getAddress();
                break;
            }
            case lyric_assembler::SymbolType::CLASS: {
                auto *classSymbol = cast_symbol_to_class(symbol);
                binding_type = lyo1::DescriptorSection::Class;
                auto address = classSymbol->getAddress();
                TU_ASSERT (address.isValid());
                binding_descriptor = address.getAddress();
                break;
            }
            case lyric_assembler::SymbolType::CONCEPT: {
                auto *conceptSymbol = cast_symbol_to_concept(symbol);
                binding_type = lyo1::DescriptorSection::Concept;
                auto address = conceptSymbol->getAddress();
                TU_ASSERT (address.isValid());
                binding_descriptor = address.getAddress();
                break;
            }
            case lyric_assembler::SymbolType::ENUM: {
                auto *enumSymbol = cast_symbol_to_enum(symbol);
                binding_type = lyo1::DescriptorSection::Enum;
                auto address = enumSymbol->getAddress();
                TU_ASSERT (address.isValid());
                binding_descriptor = address.getAddress();
                break;
            }
            case lyric_assembler::SymbolType::INSTANCE: {
                auto *instanceSymbol = cast_symbol_to_instance(symbol);
                binding_type = lyo1::DescriptorSection::Instance;
                auto address = instanceSymbol->getAddress();
                TU_ASSERT (address.isValid());
                binding_descriptor = address.getAddress();
                break;
            }
            case lyric_assembler::SymbolType::NAMESPACE: {
                auto *namespaceSymbol_ = cast_symbol_to_namespace(symbol);
                binding_type = lyo1::DescriptorSection::Namespace;
                auto address = namespaceSymbol_->getAddress();
                TU_ASSERT (address.isValid());
                binding_descriptor = address.getAddress();
                break;
            }
            case lyric_assembler::SymbolType::STATIC: {
                auto *staticSymbol = cast_symbol_to_static(symbol);
                binding_type = lyo1::DescriptorSection::Static;
                auto address = staticSymbol->getAddress();
                TU_ASSERT (address.isValid());
                binding_descriptor = address.getAddress();
                break;
            }
            case lyric_assembler::SymbolType::STRUCT: {
                auto *structSymbol = cast_symbol_to_struct(symbol);
                binding_type = lyo1::DescriptorSection::Struct;
                auto address = structSymbol->getAddress();
                TU_ASSERT (address.isValid());
                binding_descriptor = address.getAddress();
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
    const AssemblyState *assemblyState,
    flatbuffers::FlatBufferBuilder &buffer,
    NamespacesOffset &namespacesOffset,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    TU_ASSERT (assemblyState != nullptr);

    SymbolCache *symbolCache = assemblyState->symbolCache();
    TypeCache *typeCache = assemblyState->typeCache();
    std::vector<flatbuffers::Offset<lyo1::NamespaceDescriptor>> namespaces_vector;

    for (auto iterator = assemblyState->namespacesBegin(); iterator != assemblyState->namespacesEnd(); iterator++) {
        auto &namespaceSymbol = *iterator;
        TU_RETURN_IF_NOT_OK (write_namespace(namespaceSymbol, assemblyState->getLocation(),
            typeCache, symbolCache, buffer, namespaces_vector, symbols_vector));
    }

    // create the namespaces vector
    namespacesOffset = buffer.CreateVector(namespaces_vector);

    return {};
}
