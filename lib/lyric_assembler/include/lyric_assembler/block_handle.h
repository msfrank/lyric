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
        absl::flat_hash_map<lyric_common::TypeDef, ImplReference>::const_iterator implsBegin() const;
        absl::flat_hash_map<lyric_common::TypeDef, ImplReference>::const_iterator implsEnd() const;

        bool hasBinding(const std::string &name) const;
        SymbolBinding getBinding(const std::string &name) const;
        tempo_utils::Status putBinding(AbstractSymbol *symbol);

        tempo_utils::Result<lyric_common::TypeDef> resolveSingular(
            const lyric_common::SymbolPath &typePath,
            const std::vector<lyric_common::TypeDef> &typeArguments) override;

        tempo_utils::Result<lyric_common::SymbolUrl> resolveDefinition(
            const lyric_common::SymbolPath &symbolPath);
        tempo_utils::Result<lyric_common::SymbolUrl> resolveDefinition(
            const std::vector<std::string> &path);

        tempo_utils::Result<lyric_common::SymbolUrl> resolveFunction(const std::string &name);

        tempo_utils::Result<DataReference> declareVariable(
            const std::string &name,
            bool isHidden,
            const lyric_common::TypeDef &assignableType,
            bool isVariable);

        tempo_utils::Result<DataReference> declareTemporary(
            const lyric_common::TypeDef &assignableType,
            bool isVariable);

        tempo_utils::Result<DataReference> declareStatic(
            const std::string &name,
            bool isHidden,
            const lyric_common::TypeDef &assignableType,
            bool isVariable,
            bool declOnly = false);

        tempo_utils::Result<DataReference> resolveReference(const std::string &name);
        tempo_utils::Result<DataReference> resolveReference(const std::vector<std::string> &path);

        tempo_utils::Result<CallSymbol *> declareFunction(
            const std::string &name,
            bool isHidden,
            const std::vector<lyric_object::TemplateParameter> &templateParameters,
            bool declOnly = false);

        tempo_utils::Status prepareFunction(const std::string &name, CallableInvoker &invoker);

        tempo_utils::Result<ClassSymbol *> declareClass(
            const std::string &name,
            ClassSymbol *superClass,
            bool isHidden,
            const std::vector<lyric_object::TemplateParameter> &templateParameters,
            lyric_object::DeriveType derive = lyric_object::DeriveType::Any,
            bool declOnly = false);

        tempo_utils::Result<ClassSymbol *> resolveClass(const lyric_common::TypeDef &classType);

        tempo_utils::Result<ConceptSymbol *> declareConcept(
            const std::string &name,
            ConceptSymbol *superConcept,
            bool isHidden,
            const std::vector<lyric_object::TemplateParameter> &templateParameters,
            lyric_object::DeriveType derive = lyric_object::DeriveType::Any,
            bool declOnly = false);

        tempo_utils::Result<ConceptSymbol *> resolveConcept(const lyric_common::TypeDef &conceptType);

        tempo_utils::Result<EnumSymbol *> declareEnum(
            const std::string &name,
            EnumSymbol *superEnum,
            bool isHidden,
            lyric_object::DeriveType derive = lyric_object::DeriveType::Any,
            bool declOnly = false);

        tempo_utils::Result<EnumSymbol *> resolveEnum(const lyric_common::TypeDef &enumType);

        tempo_utils::Result<InstanceSymbol *> declareInstance(
            const std::string &name,
            InstanceSymbol *superInstance,
            bool isHidden,
            lyric_object::DeriveType derive = lyric_object::DeriveType::Any,
            bool declOnly = false);

        tempo_utils::Result<InstanceSymbol *> resolveInstance(const lyric_common::TypeDef &instanceType);

        tempo_utils::Result<StructSymbol *> declareStruct(
            const std::string &name,
            StructSymbol *superStruct,
            bool isHidden,
            lyric_object::DeriveType derive = lyric_object::DeriveType::Any,
            bool declOnly = false);

        tempo_utils::Result<StructSymbol *> resolveStruct(const lyric_common::TypeDef &structType);

        tempo_utils::Status useImpls(
            const DataReference &usingRef,
            const absl::flat_hash_set<lyric_common::TypeDef> &implTypes = {});
        tempo_utils::Status useImpls(
            const InstanceSymbol *usingInstance,
            const absl::flat_hash_set<lyric_common::TypeDef> &implTypes = {});

        bool hasImpl(const lyric_common::TypeDef &implType) const;
        Option<ImplReference> getImpl(const lyric_common::TypeDef &implType) const;
        tempo_utils::Result<ImplReference> resolveImpl(
            const lyric_common::TypeDef &implType,
            const std::vector<lyric_common::TypeDef> &fallbackImplTypes = {});

        tempo_utils::Result<BindingSymbol *> declareBinding(
            const std::string &name,
            bool isHidden,
            const std::vector<lyric_object::TemplateParameter> &templateParameters = {});

        tempo_utils::Result<TypenameSymbol *> declareTypename(const std::string &name, bool isHidden);

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
        absl::flat_hash_map<lyric_common::TypeDef, ImplReference> m_impls;

        tempo_utils::Result<SymbolBinding> resolveBinding(const std::vector<std::string> &path);

        tempo_utils::Result<TypenameSymbol *> checkForTypenameOrNull(
            std::string_view name,
            const lyric_common::SymbolUrl &symbolUrl);
    };
}

#endif // LYRIC_ASSEMBLER_BLOCK_HANDLE_H
