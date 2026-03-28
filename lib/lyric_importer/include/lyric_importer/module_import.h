#ifndef LYRIC_IMPORTER_MODULE_IMPORT_H
#define LYRIC_IMPORTER_MODULE_IMPORT_H

#include <lyric_common/module_location.h>
#include <lyric_object/lyric_object.h>
#include <tempo_utils/result.h>

#include <lyric_runtime/abstract_plugin.h>
#include <lyric_runtime/runtime_types.h>

namespace lyric_importer {

    // forward declarations
    class ActionImport;
    class BindingImport;
    class CallImport;
    class ClassImport;
    class ConceptImport;
    class EnumImport;
    class ExistentialImport;
    class FieldImport;
    class ImplImport;
    class InstanceImport;
    class NamespaceImport;
    class ProtocolImport;
    class StaticImport;
    class StructImport;
    class TemplateImport;
    class TypeImport;

    /**
     * thread-safe, shared module import.
     */
    class ModuleImport : public std::enable_shared_from_this<ModuleImport> {
    public:
        ModuleImport() = delete;

        static tempo_utils::Result<std::shared_ptr<ModuleImport>> create(
            const lyric_common::ModuleLocation &objectLocation,
            const lyric_object::LyricObject &object,
            const lyric_common::ModuleLocation &pluginLocation = {},
            std::shared_ptr<const lyric_runtime::AbstractPlugin> plugin = {});

        lyric_common::ModuleLocation getObjectLocation() const;
        lyric_object::LyricObject getObject() const;
        lyric_common::ModuleLocation getPluginLocation() const;
        std::shared_ptr<const lyric_runtime::AbstractPlugin> getPlugin() const;

        std::shared_ptr<ActionImport> getAction(tu_uint32 offset) const;
        std::shared_ptr<BindingImport> getBinding(tu_uint32 offset) const;
        std::shared_ptr<CallImport> getCall(tu_uint32 offset) const;
        std::shared_ptr<ClassImport> getClass(tu_uint32 offset) const;
        std::shared_ptr<ConceptImport> getConcept(tu_uint32 offset) const;
        std::shared_ptr<EnumImport> getEnum(tu_uint32 offset) const;
        std::shared_ptr<ExistentialImport> getExistential(tu_uint32 offset) const;
        std::shared_ptr<FieldImport> getField(tu_uint32 offset) const;
        std::shared_ptr<ImplImport> getImpl(tu_uint32 offset) const;
        std::shared_ptr<InstanceImport> getInstance(tu_uint32 offset) const;
        std::shared_ptr<NamespaceImport> getNamespace(tu_uint32 offset) const;
        std::shared_ptr<ProtocolImport> getProtocol(tu_uint32 offset) const;
        std::shared_ptr<StaticImport> getStatic(tu_uint32 offset) const;
        std::shared_ptr<StructImport> getStruct(tu_uint32 offset) const;
        std::shared_ptr<TemplateImport> getTemplate(tu_uint32 offset) const;
        std::shared_ptr<TypeImport> getType(tu_uint32 offset) const;

        lyric_common::SymbolUrl resolveLinkUrl(const lyric_common::SymbolUrl &linkUrl) const;

    private:
        lyric_common::ModuleLocation m_objectLocation;
        lyric_object::LyricObject m_object;
        lyric_common::ModuleLocation m_pluginLocation;
        std::shared_ptr<const lyric_runtime::AbstractPlugin> m_plugin;

        std::vector<std::shared_ptr<ActionImport>> m_importedActions;
        std::vector<std::shared_ptr<BindingImport>> m_importedBindings;
        std::vector<std::shared_ptr<CallImport>> m_importedCalls;
        std::vector<std::shared_ptr<ClassImport>> m_importedClasses;
        std::vector<std::shared_ptr<ConceptImport>> m_importedConcepts;
        std::vector<std::shared_ptr<EnumImport>> m_importedEnums;
        std::vector<std::shared_ptr<ExistentialImport>> m_importedExistentials;
        std::vector<std::shared_ptr<FieldImport>> m_importedFields;
        std::vector<std::shared_ptr<ImplImport>> m_importedImpls;
        std::vector<std::shared_ptr<InstanceImport>> m_importedInstances;
        std::vector<std::shared_ptr<NamespaceImport>> m_importedNamespaces;
        std::vector<std::shared_ptr<ProtocolImport>> m_importedProtocols;
        std::vector<std::shared_ptr<StaticImport>> m_importedStatics;
        std::vector<std::shared_ptr<StructImport>> m_importedStructs;
        std::vector<std::shared_ptr<TemplateImport>> m_importedTemplates;
        std::vector<std::shared_ptr<TypeImport>> m_importedTypes;

        ModuleImport(
            const lyric_common::ModuleLocation &objectLocation,
            const lyric_object::LyricObject &object,
            const lyric_common::ModuleLocation &pluginLocation,
            std::shared_ptr<const lyric_runtime::AbstractPlugin> plugin);
        tempo_utils::Status initialize();

        friend class ModuleCache;
    };
}

#endif // LYRIC_IMPORTER_MODULE_IMPORT_H
