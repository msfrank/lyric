#ifndef LYRIC_ASSEMBLER_CONCEPT_SYMBOL_H
#define LYRIC_ASSEMBLER_CONCEPT_SYMBOL_H

#include <absl/container/flat_hash_map.h>

#include <lyric_importer/concept_import.h>

#include "abstract_symbol.h"
#include "action_invoker.h"
#include "assembly_state.h"
#include "base_symbol.h"
#include "call_invoker.h"
#include "type_handle.h"

namespace lyric_assembler {

    struct ConceptSymbolPriv {
        lyric_object::AccessType access;
        lyric_object::DeriveType derive;
        TypeHandle *conceptType;
        TemplateHandle *conceptTemplate;
        ConceptSymbol *superConcept;
        absl::flat_hash_map<std::string, ActionMethod> actions;
        absl::flat_hash_map<lyric_common::SymbolUrl, ImplHandle *> impls;
        absl::flat_hash_set<lyric_common::TypeDef> sealedTypes;
        std::unique_ptr<BlockHandle> conceptBlock;
    };

    class ConceptSymbol : public BaseSymbol<ConceptAddress,ConceptSymbolPriv> {

    public:
        ConceptSymbol(
            const lyric_common::SymbolUrl &conceptUrl,
            lyric_object::AccessType access,
            lyric_object::DeriveType derive,
            ConceptAddress address,
            TypeHandle *conceptType,
            TemplateHandle *conceptTemplate,
            ConceptSymbol *superConcept,
            BlockHandle *parentBlock,
            AssemblyState *state);

        ConceptSymbol(
            const lyric_common::SymbolUrl &conceptUrl,
            lyric_object::AccessType access,
            lyric_object::DeriveType derive,
            ConceptAddress address,
            TypeHandle *conceptType,
            ConceptSymbol *superConcept,
            BlockHandle *parentBlock,
            AssemblyState *state);

        ConceptSymbol(
            const lyric_common::SymbolUrl &conceptUrl,
            lyric_importer::ConceptImport *conceptImport,
            AssemblyState *state);

        lyric_object::LinkageSection getLinkage() const override;

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getAssignableType() const override;
        TypeSignature getTypeSignature() const override;
        void touch() override;

        lyric_object::AccessType getAccessType() const;
        lyric_object::DeriveType getDeriveType() const;

        ConceptSymbol *superConcept() const;
        TypeHandle *conceptType() const;
        TemplateHandle *conceptTemplate() const;
        BlockHandle *conceptBlock() const;

        bool hasAction(const std::string &name) const;
        Option<ActionMethod> getAction(const std::string &name) const;
        absl::flat_hash_map<std::string, ActionMethod>::const_iterator actionsBegin() const;
        absl::flat_hash_map<std::string, ActionMethod>::const_iterator actionsEnd() const;
        tu_uint32 numActions() const;

        tempo_utils::Result<lyric_common::SymbolUrl> declareAction(
            const std::string &name,
            const std::vector<ParameterSpec> &parameterSpec,
            const Option<ParameterSpec> &restSpec,
            const std::vector<ParameterSpec> &ctxSpec,
            const lyric_parser::Assignable &returnSpec,
            lyric_object::AccessType access);

        tempo_utils::Result<ActionInvoker> resolveAction(
            const std::string &name,
            const lyric_common::TypeDef &receiverType,
            bool isReceiver = false) const;

        /*
         * concept impl management
         */
        bool hasImpl(const lyric_common::SymbolUrl &implUrl) const;
        bool hasImpl(const lyric_common::TypeDef &implType) const;
        ImplHandle *getImpl(const lyric_common::SymbolUrl &implUrl) const;
        ImplHandle *getImpl(const lyric_common::TypeDef &implType) const;
        absl::flat_hash_map<lyric_common::SymbolUrl, ImplHandle *>::const_iterator implsBegin() const;
        absl::flat_hash_map<lyric_common::SymbolUrl, ImplHandle *>::const_iterator implsEnd() const;
        tu_uint32 numImpls() const;

        tempo_utils::Result<lyric_common::TypeDef> declareImpl(const lyric_parser::Assignable &implSpec);

        /*
         * subtype tracking for sealed concept
         */
        bool hasSealedType(const lyric_common::TypeDef &sealedType) const;
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesBegin() const;
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesEnd() const;
        tempo_utils::Status putSealedType(const lyric_common::TypeDef &sealedType);

    private:
        lyric_common::SymbolUrl m_conceptUrl;
        lyric_importer::ConceptImport *m_conceptImport = nullptr;
        AssemblyState *m_state;

        ConceptSymbolPriv *load() override;
    };

    static inline const ConceptSymbol *cast_symbol_to_concept(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::CONCEPT);
        return static_cast<const ConceptSymbol *>(sym);     // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    static inline ConceptSymbol *cast_symbol_to_concept(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::CONCEPT);
        return static_cast<ConceptSymbol *>(sym);           // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_CONCEPT_SYMBOL_H
