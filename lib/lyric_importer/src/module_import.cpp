
#include <lyric_importer/action_import.h>
#include <lyric_importer/binding_import.h>
#include <lyric_importer/call_import.h>
#include <lyric_importer/class_import.h>
#include <lyric_importer/concept_import.h>
#include <lyric_importer/enum_import.h>
#include <lyric_importer/existential_import.h>
#include <lyric_importer/field_import.h>
#include <lyric_importer/impl_import.h>
#include <lyric_importer/importer_result.h>
#include <lyric_importer/instance_import.h>
#include <lyric_importer/module_import.h>
#include <lyric_importer/namespace_import.h>
#include <lyric_importer/static_import.h>
#include <lyric_importer/struct_import.h>
#include <lyric_importer/template_import.h>
#include <lyric_importer/type_import.h>


lyric_importer::ModuleImport::ModuleImport(
    const lyric_common::ModuleLocation &importLocation,
    const lyric_object::LyricObject &importObject,
    std::shared_ptr<const lyric_runtime::AbstractPlugin> importPlugin)
    : m_location(importLocation),
      m_object(importObject),
      m_plugin(std::move(importPlugin))
{
    TU_ASSERT (m_location.isValid());
    TU_ASSERT (m_object.isValid());
}

tempo_utils::Result<std::shared_ptr<lyric_importer::ModuleImport>>
lyric_importer::ModuleImport::create(
    const lyric_common::ModuleLocation &importLocation,
    const lyric_object::LyricObject &importObject,
    std::shared_ptr<const lyric_runtime::AbstractPlugin> importPlugin)
{
    auto moduleImport = std::shared_ptr<ModuleImport>(new ModuleImport(importLocation, importObject, importPlugin));
    TU_RETURN_IF_NOT_OK (moduleImport->initialize());
    return moduleImport;
}

tempo_utils::Status
lyric_importer::ModuleImport::initialize()
{
    auto object = m_object.getObject();

    m_importedActions.resize(object.numActions());
    for (int i = 0; i < object.numActions(); i++) {
        m_importedActions[i] = new ActionImport(shared_from_this(), i);
    }

    m_importedBindings.resize(object.numBindings());
    for (int i = 0; i < object.numBindings(); i++) {
        m_importedBindings[i] = new BindingImport(shared_from_this(), i);
    }

    m_importedCalls.resize(object.numCalls());
    for (int i = 0; i < object.numCalls(); i++) {
        m_importedCalls[i] = new CallImport(shared_from_this(), i);
    }

    m_importedClasses.resize(object.numClasses());
    for (int i = 0; i < object.numClasses(); i++) {
        m_importedClasses[i] = new ClassImport(shared_from_this(), i);
    }

    m_importedConcepts.resize(object.numConcepts());
    for (int i = 0; i < object.numConcepts(); i++) {
        m_importedConcepts[i] = new ConceptImport(shared_from_this(), i);
    }

    m_importedEnums.resize(object.numEnums());
    for (int i = 0; i < object.numEnums(); i++) {
        m_importedEnums[i] = new EnumImport(shared_from_this(), i);
    }

    m_importedExistentials.resize(object.numExistentials());
    for (int i = 0; i < object.numExistentials(); i++) {
        m_importedExistentials[i] = new ExistentialImport(shared_from_this(), i);
    }

    m_importedFields.resize(object.numFields());
    for (int i = 0; i < object.numFields(); i++) {
        m_importedFields[i] = new FieldImport(shared_from_this(), i);
    }

    m_importedImpls.resize(object.numImpls());
    for (int i = 0; i < object.numImpls(); i++) {
        m_importedImpls[i] = new ImplImport(shared_from_this(), i);
    }

    m_importedInstances.resize(object.numInstances());
    for (int i = 0; i < object.numInstances(); i++) {
        m_importedInstances[i] = new InstanceImport(shared_from_this(), i);
    }

    m_importedNamespaces.resize(object.numNamespaces());
    for (int i = 0; i < object.numNamespaces(); i++) {
        m_importedNamespaces[i] = new NamespaceImport(shared_from_this(), i);
    }

    m_importedStatics.resize(object.numStatics());
    for (int i = 0; i < object.numStatics(); i++) {
        m_importedStatics[i] = new StaticImport(shared_from_this(), i);
    }

    m_importedStructs.resize(object.numStructs());
    for (int i = 0; i < object.numStructs(); i++) {
        m_importedStructs[i] = new StructImport(shared_from_this(), i);
    }

    m_importedTemplates.resize(object.numTemplates());
    for (int i = 0; i < object.numTemplates(); i++) {
        m_importedTemplates[i] = new TemplateImport(shared_from_this(), i);
    }

    m_importedTypes.resize(object.numTypes());
    for (int i = 0; i < object.numTypes(); i++) {
        m_importedTypes[i] = new TypeImport(shared_from_this(), i);
    }

    return ImporterStatus::ok();
}

lyric_common::ModuleLocation
lyric_importer::ModuleImport::getLocation() const
{
    return m_location;
}

lyric_object::LyricObject
lyric_importer::ModuleImport::getObject() const
{
    return m_object;
}

std::shared_ptr<const lyric_runtime::AbstractPlugin>
lyric_importer::ModuleImport::getPlugin() const
{
    return m_plugin;
}

lyric_importer::ActionImport *
lyric_importer::ModuleImport::getAction(tu_uint32 offset) const
{
    if (offset < m_importedActions.size())
        return m_importedActions.at(offset);
    return nullptr;
}

lyric_importer::BindingImport *
lyric_importer::ModuleImport::getBinding(tu_uint32 offset) const
{
    if (offset < m_importedBindings.size())
        return m_importedBindings.at(offset);
    return nullptr;
}

lyric_importer::CallImport *
lyric_importer::ModuleImport::getCall(tu_uint32 offset) const
{
    if (offset < m_importedCalls.size())
        return m_importedCalls.at(offset);
    return nullptr;
}

lyric_importer::ClassImport *
lyric_importer::ModuleImport::getClass(tu_uint32 offset) const
{
    if (offset < m_importedClasses.size())
        return m_importedClasses.at(offset);
    return nullptr;
}

lyric_importer::ConceptImport *
lyric_importer::ModuleImport::getConcept(tu_uint32 offset) const
{
    if (offset < m_importedConcepts.size())
        return m_importedConcepts.at(offset);
    return nullptr;
}

lyric_importer::EnumImport *
lyric_importer::ModuleImport::getEnum(tu_uint32 offset) const
{
    if (offset < m_importedEnums.size())
        return m_importedEnums.at(offset);
    return nullptr;
}

lyric_importer::ExistentialImport *
lyric_importer::ModuleImport::getExistential(tu_uint32 offset) const
{
    if (offset < m_importedExistentials.size())
        return m_importedExistentials.at(offset);
    return nullptr;
}

lyric_importer::FieldImport *
lyric_importer::ModuleImport::getField(tu_uint32 offset) const
{
    if (offset < m_importedFields.size())
        return m_importedFields.at(offset);
    return nullptr;
}

lyric_importer::ImplImport *
lyric_importer::ModuleImport::getImpl(tu_uint32 offset) const
{
    if (offset < m_importedImpls.size())
        return m_importedImpls.at(offset);
    return nullptr;
}

lyric_importer::InstanceImport *
lyric_importer::ModuleImport::getInstance(tu_uint32 offset) const
{
    if (offset < m_importedInstances.size())
        return m_importedInstances.at(offset);
    return nullptr;
}

lyric_importer::NamespaceImport *
lyric_importer::ModuleImport::getNamespace(tu_uint32 offset) const
{
    if (offset < m_importedNamespaces.size())
        return m_importedNamespaces.at(offset);
    return nullptr;
}

lyric_importer::StaticImport *
lyric_importer::ModuleImport::getStatic(tu_uint32 offset) const
{
    if (offset < m_importedStatics.size())
        return m_importedStatics.at(offset);
    return nullptr;
}

lyric_importer::StructImport *
lyric_importer::ModuleImport::getStruct(tu_uint32 offset) const
{
    if (offset < m_importedStructs.size())
        return m_importedStructs.at(offset);
    return nullptr;
}

lyric_importer::TemplateImport *
lyric_importer::ModuleImport::getTemplate(tu_uint32 offset) const
{
    if (offset < m_importedTemplates.size())
        return m_importedTemplates.at(offset);
    return nullptr;
}

lyric_importer::TypeImport *
lyric_importer::ModuleImport::getType(tu_uint32 offset) const
{
    if (offset < m_importedTypes.size())
        return m_importedTypes.at(offset);
    return nullptr;
}
