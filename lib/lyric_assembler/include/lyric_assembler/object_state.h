#ifndef LYRIC_ASSEMBLER_OBJECT_STATE_H
#define LYRIC_ASSEMBLER_OBJECT_STATE_H

#include <string>
#include <vector>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/container/node_hash_map.h>

#include <lyric_common/symbol_url.h>
#include <lyric_importer/module_cache.h>
#include <lyric_importer/shortcut_resolver.h>
#include <lyric_runtime/abstract_loader.h>
#include <lyric_runtime/literal_cell.h>
#include <lyric_runtime/trap_index.h>

#include "abstract_symbol.h"
#include "assembler_result.h"
#include "assembler_types.h"

namespace lyric_assembler {

    // forward declarations
    class ActionSymbol;
    class BindingSymbol;
    class BlockHandle;
    class CallSymbol;
    class ClassSymbol;
    class ConceptSymbol;
    class EnumSymbol;
    class ExistentialSymbol;
    class FieldSymbol;
    class FundamentalCache;
    class ImplCache;
    class ImplHandle;
    class ImportCache;
    class InstanceSymbol;
    class LinkageSymbol;
    class LiteralCache;
    class LiteralHandle;
    class NamespaceSymbol;
    class ObjectPlugin;
    class ObjectRoot;
    class ProcHandle;
    class StaticSymbol;
    class StructSymbol;
    class SymbolCache;
    class TemplateHandle;
    class TypeCache;
    class TypeHandle;
    class TypenameSymbol;

    enum class ProcImportMode {
        Invalid,
        None,                       /*< Do not import any procs. */
        InlineableOnly,             /*< Import inlineable procs only. */
        All,                        /*< Import all procs. */
    };

    struct ObjectStateLimits {
    };

    struct ObjectStateOptions {
        tu_uint32 majorVersion = 0; /*< Object major version. */
        tu_uint32 minorVersion = 0; /*< Object minor version. */
        tu_uint32 patchVersion = 0; /*< Object patch version. */
        /**
         * The location of the prelude module. If not specified then the default location from
         * lyric_bootstrap is used.
         */
        lyric_common::ModuleLocation preludeLocation = {};
        /**
         * The location of any environment modules. If not empty then the vector is processed in
         * reverse order and aliases for all top level symbols in each location are inserted into
         * the internal environment block.
         */
        std::vector<lyric_common::ModuleLocation> environmentModules = {};
        /**
         * The proc import mode. If not specified then defaults to only importing procs which are
         * marked as inlineable.
         */
        ProcImportMode procImportMode = ProcImportMode::InlineableOnly;
        /**
         * Object state limits.
         */
        ObjectStateLimits limits = {};
    };

    class ObjectState {

    public:
        ObjectState(
            const lyric_common::ModuleLocation &location,
            const lyric_common::ModuleLocation &origin,
            std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
            std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
            std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver,
            const ObjectStateOptions &options = {});
        ObjectState(
            const lyric_common::ModuleLocation &location,
            const lyric_common::ModuleLocation &origin,
            std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
            std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
            std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver,
            const lyric_common::ModuleLocation &pluginLocation,
            const ObjectStateOptions &options = {});
        ~ObjectState();

        lyric_common::ModuleLocation getLocation() const;
        lyric_common::ModuleLocation getOrigin() const;
        lyric_common::ModuleLocation getPluginLocation() const;
        const ObjectStateOptions *getOptions() const;

        tempo_utils::Status load();

        tempo_utils::Result<ObjectRoot *> defineRoot();

        ObjectRoot *objectRoot() const;
        ObjectPlugin *objectPlugin() const;
        FundamentalCache *fundamentalCache() const;
        ImportCache *importCache() const;
        SymbolCache *symbolCache() const;
        LiteralCache *literalCache() const;
        TypeCache *typeCache() const;
        ImplCache *implCache() const;

        tempo_utils::Result<ActionSymbol *> appendAction(
            std::unique_ptr<ActionSymbol> &&actionSymbol,
            TypenameSymbol *existingTypename = nullptr);
        std::vector<ActionSymbol *>::const_iterator actionsBegin() const;
        std::vector<ActionSymbol *>::const_iterator actionsEnd() const;
        int numActions() const;

        tempo_utils::Result<BindingSymbol *> appendBinding(
            std::unique_ptr<BindingSymbol> &&bindingSymbol,
            TypenameSymbol *existingTypename = nullptr);
        std::vector<BindingSymbol *>::const_iterator bindingsBegin() const;
        std::vector<BindingSymbol *>::const_iterator bindingsEnd() const;
        int numBindings() const;

        tempo_utils::Result<CallSymbol *> appendCall(
            std::unique_ptr<CallSymbol> &&callSymbol,
            TypenameSymbol *existingTypename = nullptr);
        std::vector<CallSymbol *>::const_iterator callsBegin() const;
        std::vector<CallSymbol *>::const_iterator callsEnd() const;
        int numCalls() const;

        tempo_utils::Result<ClassSymbol *> appendClass(
            std::unique_ptr<ClassSymbol> &&classSymbol,
            TypenameSymbol *existingTypename = nullptr);
        std::vector<ClassSymbol *>::const_iterator classesBegin() const;
        std::vector<ClassSymbol *>::const_iterator classesEnd() const;
        int numClasses() const;

        tempo_utils::Result<ConceptSymbol *> appendConcept(
            std::unique_ptr<ConceptSymbol> &&conceptSymbol,
            TypenameSymbol *existingTypename = nullptr);
        std::vector<ConceptSymbol *>::const_iterator conceptsBegin() const;
        std::vector<ConceptSymbol *>::const_iterator conceptsEnd() const;
        int numConcepts() const;

        tempo_utils::Result<EnumSymbol *> appendEnum(
            std::unique_ptr<EnumSymbol> &&enumSymbol,
            TypenameSymbol *existingTypename = nullptr);
        std::vector<EnumSymbol *>::const_iterator enumsBegin() const;
        std::vector<EnumSymbol *>::const_iterator enumsEnd() const;
        int numEnums() const;

        tempo_utils::Result<ExistentialSymbol *> appendExistential(
            std::unique_ptr<ExistentialSymbol> &&existentialSymbol,
            TypenameSymbol *existingTypename = nullptr);
        std::vector<ExistentialSymbol *>::const_iterator existentialsBegin() const;
        std::vector<ExistentialSymbol *>::const_iterator existentialsEnd() const;
        int numExistentials() const;

        tempo_utils::Result<FieldSymbol *> appendField(
            std::unique_ptr<FieldSymbol> &&fieldSymbol,
            TypenameSymbol *existingTypename = nullptr);
        std::vector<FieldSymbol *>::const_iterator fieldsBegin() const;
        std::vector<FieldSymbol *>::const_iterator fieldsEnd() const;
        int numFields() const;

        tempo_utils::Result<InstanceSymbol *> appendInstance(
            std::unique_ptr<InstanceSymbol> &&instanceSymbol,
            TypenameSymbol *existingTypename = nullptr);
        std::vector<InstanceSymbol *>::const_iterator instancesBegin() const;
        std::vector<InstanceSymbol *>::const_iterator instancesEnd() const;
        int numInstances() const;

        tempo_utils::Result<LinkageSymbol *> appendLinkage(
            std::unique_ptr<LinkageSymbol> &&linkageSymbol,
            TypenameSymbol *existingTypename = nullptr);
        std::vector<LinkageSymbol *>::const_iterator linkagesBegin() const;
        std::vector<LinkageSymbol *>::const_iterator linkagesEnd() const;
        int numLinkages() const;

        tempo_utils::Result<NamespaceSymbol *> appendNamespace(
            std::unique_ptr<NamespaceSymbol> &&namespaceSymbol,
            TypenameSymbol *existingTypename = nullptr);
        std::vector<NamespaceSymbol *>::const_iterator namespacesBegin() const;
        std::vector<NamespaceSymbol *>::const_iterator namespacesEnd() const;
        int numNamespaces() const;

        tempo_utils::Result<StaticSymbol *> appendStatic(
            std::unique_ptr<StaticSymbol> &&staticSymbol,
            TypenameSymbol *existingTypename = nullptr);
        std::vector<StaticSymbol *>::const_iterator staticsBegin() const;
        std::vector<StaticSymbol *>::const_iterator staticsEnd() const;
        int numStatics() const;

        tempo_utils::Result<StructSymbol *> appendStruct(
            std::unique_ptr<StructSymbol> &&structSymbol,
            TypenameSymbol *existingTypename = nullptr);
        std::vector<StructSymbol *>::const_iterator structsBegin() const;
        std::vector<StructSymbol *>::const_iterator structsEnd() const;
        int numStructs() const;

        tempo_utils::Result<lyric_object::LyricObject> toObject() const;

    private:
        lyric_common::ModuleLocation m_location;
        lyric_common::ModuleLocation m_origin;
        std::shared_ptr<lyric_importer::ModuleCache> m_localModuleCache;
        std::shared_ptr<lyric_importer::ModuleCache> m_systemModuleCache;
        std::shared_ptr<lyric_importer::ShortcutResolver> m_shortcutResolver;
        lyric_common::ModuleLocation m_pluginLocation;
        ObjectStateOptions m_options;

        ObjectRoot *m_root = nullptr;
        FundamentalCache *m_fundamentalcache = nullptr;
        ImportCache *m_importcache = nullptr;
        SymbolCache *m_symbolcache = nullptr;
        LiteralCache *m_literalcache = nullptr;
        TypeCache *m_typecache = nullptr;
        ImplCache *m_implcache = nullptr;
        std::unique_ptr<ObjectPlugin> m_plugin;

        std::vector<ActionSymbol *> m_actions;
        std::vector<BindingSymbol *> m_bindings;
        std::vector<CallSymbol *> m_calls;
        std::vector<ClassSymbol *> m_classes;
        std::vector<ConceptSymbol *> m_concepts;
        std::vector<EnumSymbol *> m_enums;
        std::vector<ExistentialSymbol *> m_existentials;
        std::vector<FieldSymbol *> m_fields;
        std::vector<InstanceSymbol *> m_instances;
        std::vector<LinkageSymbol *> m_linkages;
        std::vector<NamespaceSymbol *> m_namespaces;
        std::vector<StaticSymbol *> m_statics;
        std::vector<StructSymbol *> m_structs;

        tempo_utils::Status createRoot(const lyric_common::ModuleLocation &preludeLocation);
    };
}

#endif // LYRIC_ASSEMBLER_OBJECT_STATE_H
