#ifndef LYRIC_ASSEMBLER_BLOCK_HANDLE_H
#define LYRIC_ASSEMBLER_BLOCK_HANDLE_H

#include <absl/container/flat_hash_map.h>

#include <lyric_common/symbol_url.h>
#include <lyric_runtime/literal_cell.h>
#include <tempo_tracing/span_log.h>
#include <tempo_tracing/trace_span.h>

#include "abstract_resolver.h"
#include "assembler_attrs.h"
#include "object_state.h"
#include "callable_invoker.h"
#include "function_callable.h"
#include "namespace_symbol.h"
#include "proc_builder.h"
#include "type_handle.h"

namespace lyric_assembler {

    enum class ResolveMode {
        kDefault,
        kNoStatusIfMissing,
    };

    class BlockHandle : public AbstractResolver {

    public:
        explicit BlockHandle(ObjectState *state);
        BlockHandle(BlockHandle *parentBlock, ObjectState *state);
        BlockHandle(
            const lyric_common::SymbolUrl &definition,
            BlockHandle *parentBlock);
        BlockHandle(
            ProcHandle *blockProc,
            BlockHandle *parentBlock,
            ObjectState *state);
        BlockHandle(
            const absl::flat_hash_map<std::string, SymbolBinding> &initialBindings,
            ProcHandle *blockProc,
            BlockHandle *parentBlock,
            ObjectState *state);
        BlockHandle(
            const lyric_common::SymbolUrl &definition,
            const absl::flat_hash_map<std::string, SymbolBinding> &importedBindings,
            ObjectState *state);

        NamespaceSymbol *blockNs();
        ProcHandle *blockProc();
        BlockHandle *blockParent();
        ObjectState *blockState();

        bool isImported() const;

        lyric_common::SymbolUrl getDefinition() const;

        absl::flat_hash_map<std::string, SymbolBinding>::const_iterator symbolsBegin() const;
        absl::flat_hash_map<std::string, SymbolBinding>::const_iterator symbolsEnd() const;
        absl::flat_hash_map<lyric_common::TypeDef, lyric_common::SymbolUrl>::const_iterator instancesBegin() const;
        absl::flat_hash_map<lyric_common::TypeDef, lyric_common::SymbolUrl>::const_iterator instancesEnd() const;

        bool hasBinding(const std::string &name) const;
        SymbolBinding getBinding(const std::string &name) const;

        tempo_utils::Result<lyric_assembler::SymbolBinding>
        resolveBinding(const std::vector<std::string> &path);

        tempo_utils::Result<lyric_common::TypeDef> resolveSingular(
            const lyric_common::SymbolPath &typePath,
            const std::vector<lyric_common::TypeDef> &typeArguments) override;

        tempo_utils::Result<lyric_common::SymbolUrl> resolveDefinition(
            const std::vector<std::string> &typePath);
        tempo_utils::Result<lyric_common::SymbolUrl> resolveDefinition(
            const lyric_common::SymbolPath &symbolPath);

        tempo_utils::Result<DataReference> declareVariable(
            const std::string &name,
            lyric_object::AccessType access,
            const lyric_common::TypeDef &assignableType,
            bool isVariable);

        tempo_utils::Result<DataReference> declareTemporary(
            const lyric_common::TypeDef &assignableType,
            bool isVariable);

        tempo_utils::Result<DataReference> declareStatic(
            const std::string &name,
            lyric_object::AccessType access,
            const lyric_common::TypeDef &assignableType,
            bool isVariable,
            bool declOnly = false);

        tempo_utils::Result<DataReference> resolveReference(const std::string &name);

        tempo_utils::Result<CallSymbol *> declareFunction(
            const std::string &name,
            lyric_object::AccessType access,
            const std::vector<lyric_object::TemplateParameter> &templateParameters,
            bool declOnly = false);

        tempo_utils::Status prepareFunction(const std::string &name, CallableInvoker &invoker);

        tempo_utils::Result<ClassSymbol *> declareClass(
            const std::string &name,
            ClassSymbol *superClass,
            lyric_object::AccessType access,
            const std::vector<lyric_object::TemplateParameter> &templateParameters,
            lyric_object::DeriveType derive = lyric_object::DeriveType::Any,
            bool isAbstract = false,
            bool declOnly = false);

        tempo_utils::Result<ClassSymbol *> resolveClass(const lyric_common::TypeDef &classType);

        tempo_utils::Result<ConceptSymbol *> declareConcept(
            const std::string &name,
            ConceptSymbol *superConcept,
            lyric_object::AccessType access,
            const std::vector<lyric_object::TemplateParameter> &templateParameters,
            lyric_object::DeriveType derive = lyric_object::DeriveType::Any,
            bool declOnly = false);

        tempo_utils::Result<ConceptSymbol *> resolveConcept(const lyric_common::TypeDef &conceptType);

        tempo_utils::Result<EnumSymbol *> declareEnum(
            const std::string &name,
            EnumSymbol *superEnum,
            lyric_object::AccessType access,
            lyric_object::DeriveType derive = lyric_object::DeriveType::Any,
            bool isAbstract = false,
            bool declOnly = false);

        tempo_utils::Result<EnumSymbol *> resolveEnum(const lyric_common::TypeDef &enumType);

        tempo_utils::Result<InstanceSymbol *> declareInstance(
            const std::string &name,
            InstanceSymbol *superInstance,
            lyric_object::AccessType access,
            lyric_object::DeriveType derive = lyric_object::DeriveType::Any,
            bool isAbstract = false,
            bool declOnly = false);

        tempo_utils::Result<InstanceSymbol *> resolveInstance(const lyric_common::TypeDef &instanceType);

        tempo_utils::Result<StructSymbol *> declareStruct(
            const std::string &name,
            StructSymbol *superStruct,
            lyric_object::AccessType access,
            lyric_object::DeriveType derive = lyric_object::DeriveType::Any,
            bool isAbstract = false,
            bool declOnly = false);

        tempo_utils::Result<StructSymbol *> resolveStruct(const lyric_common::TypeDef &structType);

        tempo_utils::Status useSymbol(
            const lyric_common::SymbolUrl &symbolUrl,
            const absl::flat_hash_set<lyric_common::TypeDef> &implTypes = {});

        bool hasImpl(const lyric_common::TypeDef &implType) const;
        Option<lyric_common::SymbolUrl> getImpl(const lyric_common::TypeDef &implType) const;
        tempo_utils::Result<lyric_common::SymbolUrl> resolveImpl(
            const lyric_common::TypeDef &implType,
            ResolveMode mode = ResolveMode::kDefault);

//        tempo_utils::Result<NamespaceSymbol *> declareNamespace(
//            const std::string &name,
//            lyric_object::AccessType access,
//            bool declOnly = false);

        tempo_utils::Result<SymbolBinding> declareAlias(
            const std::string &alias,
            const lyric_common::SymbolUrl &targetUrl);

        tempo_utils::Result<SymbolBinding> declareAlias(
            const std::string &alias,
            const SymbolBinding &targetBinding);

        tempo_utils::Result<SymbolBinding> declareAlias(
            const std::string &alias,
            const DataReference &targetRef);

        tempo_utils::Result<SymbolBinding> declareAlias(
            const std::string &alias,
            const DataReference &targetRef,
            const lyric_common::TypeDef &aliasType);

        tempo_utils::Result<SymbolBinding> declareAlias(
            const std::string &alias,
            const lyric_common::SymbolUrl &templateUrl,
            int placeholderIndex);

        lyric_common::SymbolUrl makeSymbolUrl(const std::string &name) const;

        void debugBindings() const;

    private:
        lyric_common::SymbolUrl m_definition;
        NamespaceSymbol *m_blockNs;
        ProcHandle *m_blockProc;
        BlockHandle *m_parentBlock;
        ObjectState *m_state;
        absl::flat_hash_map<std::string, SymbolBinding> m_bindings;
        absl::flat_hash_map<lyric_common::TypeDef, lyric_common::SymbolUrl> m_impls;

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
            auto *scopeManager = m_state->scopeManager();
            auto span = scopeManager->peekSpan();
            auto status = StatusType::forCondition(condition, span->traceId(), span->spanId(),
                messageFmt, messageArgs...);
            auto log = span->logStatus(status, absl::Now(), severity);
            log->putField(kLyricAssemblerDefinitionSymbolPath, m_definition.getSymbolPath());
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
            auto *scopeManager = m_state->scopeManager();
            auto span = scopeManager->peekSpan();
            auto status = AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                span->traceId(), span->spanId(), messageFmt, messageArgs...);
            auto log = span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
            log->putField(kLyricAssemblerDefinitionSymbolPath, m_definition.getSymbolPath());
            throw tempo_utils::StatusException(status);
        }
    };
}

#endif // LYRIC_ASSEMBLER_BLOCK_HANDLE_H
