
#include <lyric_assembler/internal/write_actions.h>
#include <lyric_assembler/internal/write_calls.h>
#include <lyric_assembler/internal/write_classes.h>
#include <lyric_assembler/internal/write_concepts.h>
#include <lyric_assembler/internal/write_enums.h>
#include <lyric_assembler/internal/write_fields.h>
#include <lyric_assembler/internal/write_impls.h>
#include <lyric_assembler/internal/write_imports.h>
#include <lyric_assembler/internal/write_instances.h>
#include <lyric_assembler/internal/write_links.h>
#include <lyric_assembler/internal/write_literals.h>
#include <lyric_assembler/internal/write_namespaces.h>
#include <lyric_assembler/internal/write_object.h>
#include <lyric_assembler/internal/write_statics.h>
#include <lyric_assembler/internal/write_structs.h>
#include <lyric_assembler/internal/write_templates.h>
#include <lyric_assembler/internal/write_types.h>
#include <lyric_assembler/undeclared_symbol.h>
#include <tempo_utils/memory_bytes.h>

tempo_utils::Result<lyric_object::LyricObject>
lyric_assembler::internal::write_object(const AssemblyState *assemblyState)
{
    TU_ASSERT (assemblyState != nullptr);

    flatbuffers::FlatBufferBuilder buffer;

    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> symbols_vector;
    std::vector<flatbuffers::Offset<lyo1::PluginDescriptor>> plugins_vector;
    std::vector<uint8_t> bytecode;

    lyo1::ObjectVersion version = lyo1::ObjectVersion::Version1;

    auto *options = assemblyState->getOptions();
    auto *importCache = assemblyState->importCache();
    auto *literalCache = assemblyState->literalCache();
    auto *typeCache = assemblyState->typeCache();
    auto *symbolCache = assemblyState->symbolCache();

    // serialize array of action descriptors
    internal::ActionsOffset actionsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_actions(assemblyState, buffer, actionsOffset, symbols_vector));

    // serialize array of call descriptors
    internal::CallsOffset callsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_calls(assemblyState, buffer, callsOffset, symbols_vector, bytecode));

    // serialize array of class descriptors
    internal::ClassesOffset classesOffset;
    TU_RETURN_IF_NOT_OK (internal::write_classes(assemblyState, buffer, classesOffset, symbols_vector));

    // serialize array of concept descriptors
    internal::ConceptsOffset conceptsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_concepts(assemblyState, buffer, conceptsOffset, symbols_vector));

    // serialize array of enum descriptors
    internal::EnumsOffset enumsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_enums(assemblyState, buffer, enumsOffset, symbols_vector));

    // serialize array of field descriptors
    internal::FieldsOffset fieldsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_fields(assemblyState, buffer, fieldsOffset, symbols_vector));

    // serialize array of impl descriptors
    internal::ImplsOffset implsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_impls(assemblyState, buffer, implsOffset));

    // serialize array of import descriptors
    internal::ImportsOffset importsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_imports(importCache, buffer, importsOffset));

    // serialize array of instance descriptors
    internal::InstancesOffset instancesOffset;
    TU_RETURN_IF_NOT_OK (internal::write_instances(assemblyState, buffer, instancesOffset, symbols_vector));

    // serialize arrays of link descriptors
    internal::LinksOffset linksOffset;
    TU_RETURN_IF_NOT_OK (internal::write_links(importCache, buffer, linksOffset));

    // serialize array of literal descriptors
    internal::LiteralsOffset literalsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_literals(literalCache, buffer, literalsOffset));

    // serialize array of namespace descriptors
    internal::NamespacesOffset namespacesOffset;
    TU_RETURN_IF_NOT_OK (internal::write_namespaces(assemblyState, buffer, namespacesOffset, symbols_vector));

    // serialize array of static descriptors
    internal::StaticsOffset staticsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_statics(assemblyState, buffer, staticsOffset, symbols_vector));

    // serialize array of struct descriptors
    internal::StructsOffset structsOffset;
    TU_RETURN_IF_NOT_OK (internal::write_structs(assemblyState, buffer, structsOffset, symbols_vector));

    // serialize array of template descriptors
    internal::TemplatesOffset templatesOffset;
    TU_RETURN_IF_NOT_OK (internal::write_templates(typeCache, buffer, templatesOffset));

    // serialize array of type descriptors
    internal::TypesOffset typesOffset;
    TU_RETURN_IF_NOT_OK (internal::write_types(typeCache, symbolCache, buffer, typesOffset));

    // serialize array of undecl descriptors
    for (auto iterator = assemblyState->undeclaredBegin(); iterator != assemblyState->undeclaredEnd(); iterator++) {
        auto &undeclSymbol = *iterator;

        auto staticPathString = undeclSymbol->getSymbolUrl().getSymbolPath().toString();
        auto fb_fullyQualifiedName = buffer.CreateSharedString(staticPathString);
        auto section = internal::linkage_to_descriptor_section(undeclSymbol->getLinkage());
        if (section == lyo1::DescriptorSection::Invalid)
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "invalid undeclared symbol {}: could not parse linkage", staticPathString);

        symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, fb_fullyQualifiedName,
            section, lyric_runtime::INVALID_ADDRESS_U32));
    }

    // serialize array of plugin descriptors
    for (auto iterator = options->pluginsMap.cbegin(); iterator != options->pluginsMap.cend(); iterator++) {
        auto fb_pluginPlatform = buffer.CreateSharedString(iterator->first);
        auto fb_pluginPath = buffer.CreateSharedString(iterator->second);
        plugins_vector.push_back(lyo1::CreatePluginDescriptor(buffer, fb_pluginPath, fb_pluginPlatform));
    }

    // serialize vectors
    auto symbolsOffset = buffer.CreateVectorOfSortedTables(&symbols_vector);
    auto pluginsOffset = buffer.CreateVector(plugins_vector);
    auto bytecodeOffset = buffer.CreateVector(bytecode);

    // build assembly from buffer
    lyo1::ObjectBuilder objectBuilder(buffer);

    // set abi and assembly version
    objectBuilder.add_abi(version);
    objectBuilder.add_version_major(options->majorVersion);
    objectBuilder.add_version_minor(options->minorVersion);
    objectBuilder.add_version_patch(options->patchVersion);

    objectBuilder.add_actions(actionsOffset);
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
    objectBuilder.add_plugins(pluginsOffset);
    objectBuilder.add_statics(staticsOffset);
    objectBuilder.add_structs(structsOffset);
    objectBuilder.add_symbols(symbolsOffset);
    objectBuilder.add_templates(templatesOffset);
    objectBuilder.add_types(typesOffset);

    objectBuilder.add_bytecode(bytecodeOffset);

    // serialize assembly and mark the buffer as finished
    auto object = objectBuilder.Finish();
    buffer.Finish(object, lyo1::ObjectIdentifier());

    // copy the flatbuffer into our own byte array and instantiate object
    auto bytes = tempo_utils::MemoryBytes::copy(buffer.GetBufferSpan());
    return lyric_object::LyricObject(bytes);
}
