#ifndef LYRIC_IMPORTER_MODULE_IMPORT_H
#define LYRIC_IMPORTER_MODULE_IMPORT_H

#include <lyric_common/module_location.h>
#include <lyric_object/lyric_object.h>
#include <tempo_utils/result.h>

namespace lyric_importer {

    // forward declarations
    class ActionImport;
    class CallImport;
    class ClassImport;
    class ConceptImport;
    class EnumImport;
    class ExistentialImport;
    class FieldImport;
    class ImplImport;
    class InstanceImport;
    class NamespaceImport;
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
            const lyric_common::ModuleLocation &importLocation,
            const lyric_object::LyricObject &importObject);

        lyric_common::ModuleLocation getLocation() const;
        lyric_object::LyricObject getObject() const;

        ActionImport *getAction(tu_uint32 offset) const;
        CallImport *getCall(tu_uint32 offset) const;
        ClassImport *getClass(tu_uint32 offset) const;
        ConceptImport *getConcept(tu_uint32 offset) const;
        EnumImport *getEnum(tu_uint32 offset) const;
        ExistentialImport *getExistential(tu_uint32 offset) const;
        FieldImport *getField(tu_uint32 offset) const;
        ImplImport *getImpl(tu_uint32 offset) const;
        InstanceImport *getInstance(tu_uint32 offset) const;
        NamespaceImport *getNamespace(tu_uint32 offset) const;
        StaticImport *getStatic(tu_uint32 offset) const;
        StructImport *getStruct(tu_uint32 offset) const;
        TypeImport *getType(tu_uint32 offset) const;
        TemplateImport *getTemplate(tu_uint32 offset) const;

    private:
        lyric_common::ModuleLocation m_location;
        lyric_object::LyricObject m_object;
        std::vector<ActionImport *> m_importedActions;
        std::vector<CallImport *> m_importedCalls;
        std::vector<ClassImport *> m_importedClasses;
        std::vector<ConceptImport *> m_importedConcepts;
        std::vector<EnumImport *> m_importedEnums;
        std::vector<ExistentialImport *> m_importedExistentials;
        std::vector<FieldImport *> m_importedFields;
        std::vector<ImplImport *> m_importedImpls;
        std::vector<InstanceImport *> m_importedInstances;
        std::vector<NamespaceImport *> m_importedNamespaces;
        std::vector<StaticImport *> m_importedStatics;
        std::vector<StructImport *> m_importedStructs;
        std::vector<TemplateImport *> m_importedTemplates;
        std::vector<TypeImport *> m_importedTypes;

        ModuleImport(
            const lyric_common::ModuleLocation &importLocation,
            const lyric_object::LyricObject &importObject);
        tempo_utils::Status initialize();

        friend class ModuleCache;
    };

}

#endif // LYRIC_IMPORTER_MODULE_IMPORT_H
