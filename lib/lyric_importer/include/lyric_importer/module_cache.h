#ifndef LYRIC_IMPORTER_MODULE_CACHE_H
#define LYRIC_IMPORTER_MODULE_CACHE_H

#include <lyric_common/module_location.h>
#include <lyric_object/lyric_object.h>
#include <lyric_runtime/abstract_loader.h>
#include <tempo_utils/result.h>

#include "action_import.h"
#include "binding_import.h"
#include "call_import.h"
#include "existential_import.h"
#include "field_import.h"
#include "importer_types.h"
#include "module_import.h"
#include "namespace_import.h"
#include "static_import.h"

namespace lyric_importer {

    /**
     * thread-safe, shared module cache.
     */
    class ModuleCache : public std::enable_shared_from_this<ModuleCache> {
    public:
        ~ModuleCache();

        static std::shared_ptr<ModuleCache> create(
            std::shared_ptr<lyric_runtime::AbstractLoader> loader);

        std::shared_ptr<lyric_runtime::AbstractLoader> getLoader() const;

        bool hasModule(const lyric_common::ModuleLocation &location) const;
        std::shared_ptr<ModuleImport> getModule(const lyric_common::ModuleLocation &location) const;

        tempo_utils::Result<std::shared_ptr<ModuleImport>> insertModule(
            const lyric_common::ModuleLocation &location,
            const lyric_object::LyricObject &object);

        tempo_utils::Result<std::shared_ptr<ModuleImport>> importModule(
            const lyric_common::ModuleLocation &location);

        tempo_utils::Result<ActionImport *> getAction(const lyric_common::SymbolUrl &actionUrl);
        tempo_utils::Result<BindingImport *> getBinding(const lyric_common::SymbolUrl &bindingUrl);
        tempo_utils::Result<CallImport *> getCall(const lyric_common::SymbolUrl &callUrl);
        tempo_utils::Result<ClassImport *> getClass(const lyric_common::SymbolUrl &classUrl);
        tempo_utils::Result<ConceptImport *> getConcept(const lyric_common::SymbolUrl &conceptUrl);
        tempo_utils::Result<EnumImport *> getEnum(const lyric_common::SymbolUrl &enumUrl);
        tempo_utils::Result<ExistentialImport *> getExistential(const lyric_common::SymbolUrl &existentialUrl);
        tempo_utils::Result<FieldImport *> getField(const lyric_common::SymbolUrl &fieldUrl);
        tempo_utils::Result<InstanceImport *> getInstance(const lyric_common::SymbolUrl &instanceUrl);
        tempo_utils::Result<NamespaceImport *> getNamespace(const lyric_common::SymbolUrl &namespaceUrl);
        tempo_utils::Result<StaticImport *> getStatic(const lyric_common::SymbolUrl &staticUrl);
        tempo_utils::Result<StructImport *> getStruct(const lyric_common::SymbolUrl &structUrl);

    private:
        absl::Mutex *m_lock;
        std::shared_ptr<lyric_runtime::AbstractLoader> m_loader;
        absl::flat_hash_map<
            lyric_common::ModuleLocation,
            std::shared_ptr<ModuleImport>> m_moduleImports ABSL_GUARDED_BY(m_lock);

        explicit ModuleCache(std::shared_ptr<lyric_runtime::AbstractLoader> importerLoader);
    };

}

#endif // LYRIC_IMPORTER_MODULE_CACHE_H
