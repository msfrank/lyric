#ifndef LYRIC_ASSEMBLER_OBJECT_STATE_H
#define LYRIC_ASSEMBLER_OBJECT_STATE_H

#include <string>
#include <vector>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/container/node_hash_map.h>

#include <lyric_common/symbol_url.h>
#include <lyric_importer/module_cache.h>
#include <lyric_runtime/abstract_loader.h>
#include <lyric_runtime/literal_cell.h>
#include <lyric_runtime/trap_index.h>
#include <tempo_tracing/scope_manager.h>

#include "abstract_symbol.h"
#include "assembler_result.h"
#include "assembler_types.h"
#include "assembler_tracer.h"

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
    class UndeclaredSymbol;

    enum class ProcImportMode {
        Invalid,
        None,
        InlineableOnly,
        All,
    };

    struct ObjectStateOptions {
        tu_uint32 majorVersion = 0;
        tu_uint32 minorVersion = 0;
        tu_uint32 patchVersion = 0;
        lyric_common::ModuleLocation preludeLocation = {};
        ProcImportMode procImportMode = ProcImportMode::InlineableOnly;
    };

    class ObjectState {

    public:
        ObjectState(
            const lyric_common::ModuleLocation &location,
            std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
            std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
            tempo_tracing::ScopeManager *scopeManager,
            const ObjectStateOptions &options = {});
        ObjectState(
            const lyric_common::ModuleLocation &location,
            std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
            std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
            tempo_tracing::ScopeManager *scopeManager,
            const lyric_common::ModuleLocation &pluginLocation,
            const ObjectStateOptions &options = {});
        ~ObjectState();

        lyric_common::ModuleLocation getLocation() const;
        tempo_tracing::ScopeManager *scopeManager() const;
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

        tempo_utils::Status appendNamespace(NamespaceSymbol *namespaceSymbol);
        std::vector<NamespaceSymbol *>::const_iterator namespacesBegin() const;
        std::vector<NamespaceSymbol *>::const_iterator namespacesEnd() const;
        int numNamespaces() const;

        tempo_utils::Status appendExistential(ExistentialSymbol *existentialSymbol);
        std::vector<ExistentialSymbol *>::const_iterator existentialsBegin() const;
        std::vector<ExistentialSymbol *>::const_iterator existentialsEnd() const;
        int numExistentials() const;

        tempo_utils::Status appendStatic(StaticSymbol *staticSymbol);
        std::vector<StaticSymbol *>::const_iterator staticsBegin() const;
        std::vector<StaticSymbol *>::const_iterator staticsEnd() const;
        int numStatics() const;

        tempo_utils::Status appendField(FieldSymbol *fieldSymbol);
        std::vector<FieldSymbol *>::const_iterator fieldsBegin() const;
        std::vector<FieldSymbol *>::const_iterator fieldsEnd() const;
        int numFields() const;

        tempo_utils::Status appendAction(ActionSymbol *actionSymbol);
        std::vector<ActionSymbol *>::const_iterator actionsBegin() const;
        std::vector<ActionSymbol *>::const_iterator actionsEnd() const;
        int numActions() const;

        tempo_utils::Status appendCall(CallSymbol *callSymbol);
        std::vector<CallSymbol *>::const_iterator callsBegin() const;
        std::vector<CallSymbol *>::const_iterator callsEnd() const;
        int numCalls() const;

        tempo_utils::Status appendConcept(ConceptSymbol *conceptSymbol);
        std::vector<ConceptSymbol *>::const_iterator conceptsBegin() const;
        std::vector<ConceptSymbol *>::const_iterator conceptsEnd() const;
        int numConcepts() const;

        tempo_utils::Status appendClass(ClassSymbol *classSymbol);
        std::vector<ClassSymbol *>::const_iterator classesBegin() const;
        std::vector<ClassSymbol *>::const_iterator classesEnd() const;
        int numClasses() const;

        tempo_utils::Status appendStruct(StructSymbol *structSymbol);
        std::vector<StructSymbol *>::const_iterator structsBegin() const;
        std::vector<StructSymbol *>::const_iterator structsEnd() const;
        int numStructs() const;

        tempo_utils::Status appendInstance(InstanceSymbol *instanceSymbol);
        std::vector<InstanceSymbol *>::const_iterator instancesBegin() const;
        std::vector<InstanceSymbol *>::const_iterator instancesEnd() const;
        int numInstances() const;

        tempo_utils::Status appendEnum(EnumSymbol *enumSymbol);
        std::vector<EnumSymbol *>::const_iterator enumsBegin() const;
        std::vector<EnumSymbol *>::const_iterator enumsEnd() const;
        int numEnums() const;

        tempo_utils::Status appendBinding(BindingSymbol *bindingSymbol);
        std::vector<BindingSymbol *>::const_iterator bindingsBegin() const;
        std::vector<BindingSymbol *>::const_iterator bindingsEnd() const;
        int numBindings() const;

        tempo_utils::Status appendUndeclared(UndeclaredSymbol *undeclaredSymbol);
        std::vector<UndeclaredSymbol *>::const_iterator undeclaredBegin() const;
        std::vector<UndeclaredSymbol *>::const_iterator undeclaredEnd() const;
        int numUndeclared() const;

        tempo_utils::Result<lyric_object::LyricObject> toObject() const;

    private:
        lyric_common::ModuleLocation m_location;
        std::shared_ptr<lyric_importer::ModuleCache> m_localModuleCache;
        std::shared_ptr<lyric_importer::ModuleCache> m_systemModuleCache;
        tempo_tracing::ScopeManager *m_scopeManager;
        lyric_common::ModuleLocation m_pluginLocation;
        ObjectStateOptions m_options;

        ObjectRoot *m_root = nullptr;
        AssemblerTracer *m_tracer = nullptr;
        FundamentalCache *m_fundamentalcache = nullptr;
        ImportCache *m_importcache = nullptr;
        SymbolCache *m_symbolcache = nullptr;
        LiteralCache *m_literalcache = nullptr;
        TypeCache *m_typecache = nullptr;
        ImplCache *m_implcache = nullptr;
        std::unique_ptr<ObjectPlugin> m_plugin;

        std::vector<BindingSymbol *> m_bindings;
        std::vector<NamespaceSymbol *> m_namespaces;
        std::vector<ExistentialSymbol *> m_existentials;
        std::vector<StaticSymbol *> m_statics;
        std::vector<ActionSymbol *> m_actions;
        std::vector<CallSymbol *> m_calls;
        std::vector<FieldSymbol *> m_fields;
        std::vector<ConceptSymbol *> m_concepts;
        std::vector<ClassSymbol *> m_classes;
        std::vector<StructSymbol *> m_structs;
        std::vector<InstanceSymbol *> m_instances;
        std::vector<EnumSymbol *> m_enums;
        std::vector<UndeclaredSymbol *> m_undecls;

        tempo_utils::Status createRoot(const lyric_common::ModuleLocation &preludeLocation);

    public:
        /**
         *
         * @tparam Args
         * @param condition
         * @param severity
         * @param messageFmt
         * @param messageArgs
         * @return
         */
        template <typename ConditionType, typename... Args,
            typename StatusType = typename tempo_utils::ConditionTraits<ConditionType>::StatusType>
        StatusType logAndContinue(
            ConditionType condition,
            tempo_tracing::LogSeverity severity,
            fmt::string_view messageFmt = {},
            Args... messageArgs) const
        {
            auto span = m_scopeManager->peekSpan();
            auto status = StatusType::forCondition(condition, span->traceId(), span->spanId(),
                messageFmt, messageArgs...);
            span->logStatus(status, absl::Now(), severity);
            return status;
        }

        /**
         *
         * @tparam Args
         * @param token
         * @param messageFmt
         * @param args
         */
        template <typename... Args>
        void throwAssemblerInvariant [[noreturn]] (fmt::string_view messageFmt = {}, Args... messageArgs) const
        {
            auto span = m_scopeManager->peekSpan();
            auto status = AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                span->traceId(), span->spanId(), messageFmt, messageArgs...);
            span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
            throw tempo_utils::StatusException(status);
        }
    };
}

#endif // LYRIC_ASSEMBLER_OBJECT_STATE_H
