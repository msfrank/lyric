#ifndef LYRIC_ASSEMBLER_BLOCK_HANDLE_H
#define LYRIC_ASSEMBLER_BLOCK_HANDLE_H

#include <absl/container/flat_hash_map.h>

#include <lyric_common/symbol_url.h>
#include <lyric_parser/assignable.h>
#include <lyric_parser/node_walker.h>
#include <lyric_runtime/literal_cell.h>
#include <tempo_tracing/span_log.h>
#include <tempo_tracing/trace_span.h>

#include "abstract_resolver.h"
#include "assembly_state.h"
#include "call_invoker.h"
#include "namespace_symbol.h"
#include "type_handle.h"

namespace lyric_assembler {

    enum class ResolveMode {
        kDefault,
        kNoStatusIfMissing,
    };

    class BlockHandle : public AbstractResolver {

    public:
        BlockHandle(
            ProcHandle *blockProc,
            CodeBuilder *blockCode,
            AssemblyState *state,
            bool isRoot);
        BlockHandle(
            const lyric_common::SymbolUrl &definition,
            BlockHandle *parentBlock,
            bool isRoot);
        BlockHandle(
            NamespaceSymbol *blockNs,
            ProcHandle *blockProc,
            CodeBuilder *blockCode,
            BlockHandle *parentBlock,
            AssemblyState *state,
            bool isRoot = false);
        BlockHandle(
            ProcHandle *blockProc,
            CodeBuilder *blockCode,
            BlockHandle *parentBlock,
            AssemblyState *state);
        BlockHandle(
            const absl::flat_hash_map<std::string, SymbolBinding> &initialVars,
            ProcHandle *blockProc,
            CodeBuilder *blockCode,
            BlockHandle *parentBlock,
            AssemblyState *state);
        BlockHandle(
            const lyric_common::SymbolUrl &definition,
            const absl::flat_hash_map<std::string, SymbolBinding> &importedVars,
            AssemblyState *state);

        NamespaceSymbol *blockNs();
        ProcHandle *blockProc();
        CodeBuilder *blockCode();
        BlockHandle *blockParent();
        AssemblyState *blockState();
        bool isRoot() const;

        bool isImported() const;

        lyric_common::SymbolUrl getDefinition() const;

        absl::flat_hash_map<std::string, SymbolBinding>::const_iterator symbolsBegin() const;
        absl::flat_hash_map<std::string, SymbolBinding>::const_iterator symbolsEnd() const;
        absl::flat_hash_map<lyric_common::TypeDef, lyric_common::SymbolUrl>::const_iterator instancesBegin() const;
        absl::flat_hash_map<lyric_common::TypeDef, lyric_common::SymbolUrl>::const_iterator instancesEnd() const;

        tempo_utils::Result<lyric_common::TypeDef> resolveSingular(
            const lyric_parser::Assignable &assignableSpec);

        tempo_utils::Result<lyric_common::TypeDef> resolveAssignable(
            const lyric_parser::Assignable &assignableSpec) override;

        tempo_utils::Result<lyric_common::SymbolUrl> resolveDefinition(
            const std::vector<std::string> &typePath);
        tempo_utils::Result<lyric_common::SymbolUrl> resolveDefinition(
            const lyric_common::SymbolPath &symbolPath);
        tempo_utils::Result<lyric_common::SymbolUrl> resolveDefinition(
            const lyric_runtime::LiteralCell &literalCell);

        tempo_utils::Result<SymbolBinding> declareVariable(
            const std::string &name,
            const lyric_common::TypeDef &assignableType,
            lyric_parser::BindingType binding);

        tempo_utils::Result<SymbolBinding> declareTemporary(
            const lyric_common::TypeDef &assignableType,
            lyric_parser::BindingType binding);

        tempo_utils::Result<SymbolBinding> declareStatic(
            const std::string &name,
            const lyric_common::TypeDef &assignableType,
            lyric_parser::BindingType binding,
            bool declOnly = false);

        bool hasBinding(const std::string &name) const;
        SymbolBinding getBinding(const std::string &name) const;
        tempo_utils::Result<SymbolBinding> resolveBinding(const std::string &name);

        tempo_utils::Status load(const SymbolBinding &var);
        tempo_utils::Status store(const SymbolBinding &var);

        tempo_utils::Result<lyric_common::SymbolUrl> declareFunction(
            const std::string &name,
            const std::vector<ParameterSpec> &parameterSpec,
            const Option<ParameterSpec> &restSpec,
            const std::vector<ParameterSpec> &ctxSpec,
            const lyric_parser::Assignable &returnSpec,
            lyric_object::AccessType access,
            const std::vector<lyric_object::TemplateParameter> &templateParameters,
            bool declOnly = false);

        tempo_utils::Result<CallInvoker> resolveFunction(const std::string &name);

        tempo_utils::Result<CallInvoker> resolveExtension(
            const lyric_common::TypeDef &receiverType,
            const std::string &name);

        tempo_utils::Result<lyric_common::SymbolUrl> declareStruct(
            const std::string &name,
            StructSymbol *superStruct,
            lyric_object::AccessType access,
            lyric_object::DeriveType derive = lyric_object::DeriveType::Any,
            bool isAbstract = false,
            bool declOnly = false);

        tempo_utils::Result<lyric_common::SymbolUrl> resolveStruct(
            const lyric_parser::Assignable &structSpec);

        tempo_utils::Result<lyric_common::SymbolUrl> declareClass(
            const std::string &name,
            ClassSymbol *superClass,
            lyric_object::AccessType access,
            const std::vector<lyric_object::TemplateParameter> &templateParameters,
            lyric_object::DeriveType derive = lyric_object::DeriveType::Any,
            bool isAbstract = false,
            bool declOnly = false);

        tempo_utils::Result<lyric_common::SymbolUrl> resolveClass(
            const lyric_parser::Assignable &classSpec);

        tempo_utils::Result<lyric_common::SymbolUrl> declareConcept(
            const std::string &name,
            ConceptSymbol *superConcept,
            lyric_object::AccessType access,
            const std::vector<lyric_object::TemplateParameter> &templateParameters,
            lyric_object::DeriveType derive = lyric_object::DeriveType::Any,
            bool declOnly = false);

        tempo_utils::Result<lyric_common::SymbolUrl> resolveConcept(
            const lyric_parser::Assignable &conceptSpec);

        tempo_utils::Result<lyric_common::SymbolUrl> declareEnum(
            const std::string &name,
            EnumSymbol *superEnum,
            lyric_object::AccessType access,
            lyric_object::DeriveType derive = lyric_object::DeriveType::Any,
            bool isAbstract = false,
            bool declOnly = false);

        tempo_utils::Result<SymbolBinding> resolveEnum(
            const lyric_parser::Assignable &enumSpec);

        tempo_utils::Result<lyric_common::SymbolUrl> declareInstance(
            const std::string &name,
            InstanceSymbol *superInstance,
            lyric_object::AccessType access,
            lyric_object::DeriveType derive = lyric_object::DeriveType::Any,
            bool isAbstract = false,
            bool declOnly = false);

        tempo_utils::Result<SymbolBinding> resolveInstance(
            const lyric_parser::Assignable &instanceSpec);

        tempo_utils::Status useSymbol(
            const lyric_common::SymbolUrl &symbolUrl,
            const absl::flat_hash_set<lyric_common::TypeDef> &implTypes = {});

        bool hasImpl(const lyric_common::TypeDef &implType) const;
        Option<lyric_common::SymbolUrl> getImpl(const lyric_common::TypeDef &implType) const;
        tempo_utils::Result<lyric_common::SymbolUrl> resolveImpl(
            const lyric_common::TypeDef &implType,
            ResolveMode mode = ResolveMode::kDefault);

        tempo_utils::Result<lyric_common::SymbolUrl> declareNamespace(
            const std::string &name,
            lyric_object::AccessType access);

        tempo_utils::Result<SymbolBinding> declareAlias(
            const std::string &alias,
            const lyric_common::SymbolUrl &target,
            const lyric_common::TypeDef &aliasType = {});

        tempo_utils::Result<SymbolBinding> declareAlias(
            const std::string &alias,
            const SymbolBinding &target,
            const lyric_common::TypeDef &aliasType = {});

        lyric_common::SymbolUrl makeSymbolUrl(const std::string &name) const;

    private:
        lyric_common::SymbolUrl m_definition;
        NamespaceSymbol *m_blockNs;
        ProcHandle *m_blockProc;
        CodeBuilder *m_blockCode;
        BlockHandle *m_parentBlock;
        AssemblyState *m_state;
        bool m_isRoot;
        absl::flat_hash_map<std::string, SymbolBinding> m_vars;
        absl::flat_hash_map<lyric_common::TypeDef, lyric_common::SymbolUrl> m_impls;

    public:
        /**
         *
         * @tparam Args
         * @param walker
         * @param condition
         * @param severity
         * @param messageFmt
         * @param messageArgs
         * @return
         */
        template <typename ConditionType, typename... Args,
            typename StatusType = typename tempo_utils::ConditionTraits<ConditionType>::StatusType>
        StatusType logAndContinue(
            lyric_parser::NodeWalker walker,
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
            log->putField(lyric_parser::kLyricParserIdentifier, m_definition.getSymbolPath().toString());
            log->putField(lyric_parser::kLyricParserLineNumber, static_cast<tu_int64>(walker.getLineNumber()));
            log->putField(lyric_parser::kLyricParserColumnNumber, static_cast<tu_int64>(walker.getColumnNumber()));
            log->putField(lyric_parser::kLyricParserFileOffset, static_cast<tu_int64>(walker.getFileOffset()));
            log->putField(lyric_parser::kLyricParserTextSpan, static_cast<tu_int64>(walker.getTextSpan()));
            return status;
        }

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
            log->putField(lyric_parser::kLyricParserIdentifier, m_definition.getSymbolPath().toString());
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
        void throwSyntaxError [[noreturn]] (
            lyric_parser::NodeWalker walker,
            fmt::string_view messageFmt = {},
            Args... messageArgs) const
        {
            auto *scopeManager = m_state->scopeManager();
            auto span = scopeManager->peekSpan();
            auto status = AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
                span->traceId(), span->spanId(), messageFmt, messageArgs...);
            auto log = span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
            log->putField(lyric_parser::kLyricParserIdentifier, m_definition.getSymbolPath().toString());
            log->putField(lyric_parser::kLyricParserLineNumber, static_cast<tu_int64>(walker.getLineNumber()));
            log->putField(lyric_parser::kLyricParserColumnNumber, static_cast<tu_int64>(walker.getColumnNumber()));
            log->putField(lyric_parser::kLyricParserFileOffset, static_cast<tu_int64>(walker.getFileOffset()));
            log->putField(lyric_parser::kLyricParserTextSpan, static_cast<tu_int64>(walker.getTextSpan()));
            throw tempo_utils::StatusException(status);
        }
        /**
         *
         * @tparam Args
         * @param token
         * @param messageFmt
         * @param args
         */
        template <typename... Args>
        void throwAssemblerInvariant [[noreturn]] (
            lyric_parser::NodeWalker walker,
            fmt::string_view messageFmt = {},
            Args... messageArgs) const
        {
            auto *scopeManager = m_state->scopeManager();
            auto span = scopeManager->peekSpan();
            auto status = AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                span->traceId(), span->spanId(), messageFmt, messageArgs...);
            auto log = span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
            log->putField(lyric_parser::kLyricParserIdentifier, m_definition.getSymbolPath().toString());
            log->putField(lyric_parser::kLyricParserLineNumber, static_cast<tu_int64>(walker.getLineNumber()));
            log->putField(lyric_parser::kLyricParserColumnNumber, static_cast<tu_int64>(walker.getColumnNumber()));
            log->putField(lyric_parser::kLyricParserFileOffset, static_cast<tu_int64>(walker.getFileOffset()));
            log->putField(lyric_parser::kLyricParserTextSpan, static_cast<tu_int64>(walker.getTextSpan()));
            throw tempo_utils::StatusException(status);
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
            log->putField(lyric_parser::kLyricParserIdentifier, m_definition.getSymbolPath().toString());
            throw tempo_utils::StatusException(status);
        }
    };
}

#endif // LYRIC_ASSEMBLER_BLOCK_HANDLE_H
