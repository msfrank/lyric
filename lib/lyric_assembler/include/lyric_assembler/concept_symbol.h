#ifndef LYRIC_ASSEMBLER_CONCEPT_SYMBOL_H
#define LYRIC_ASSEMBLER_CONCEPT_SYMBOL_H

#include <absl/container/flat_hash_map.h>

#include <lyric_importer/concept_import.h>

#include "abstract_symbol.h"
#include "action_callable.h"
#include "object_state.h"
#include "base_symbol.h"
#include "impl_handle.h"
#include "type_handle.h"

namespace lyric_assembler {

    struct ConceptSymbolPriv {
        lyric_object::AccessType access;
        lyric_object::DeriveType derive;
        bool isDeclOnly;
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
            bool isDeclOnly,
            BlockHandle *parentBlock,
            ObjectState *state);

        ConceptSymbol(
            const lyric_common::SymbolUrl &conceptUrl,
            lyric_object::AccessType access,
            lyric_object::DeriveType derive,
            ConceptAddress address,
            TypeHandle *conceptType,
            ConceptSymbol *superConcept,
            bool isDeclOnly,
            BlockHandle *parentBlock,
            ObjectState *state);

        ConceptSymbol(
            const lyric_common::SymbolUrl &conceptUrl,
            lyric_importer::ConceptImport *conceptImport,
            ObjectState *state);

        lyric_object::LinkageSection getLinkage() const override;

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getAssignableType() const override;
        TypeSignature getTypeSignature() const override;
        void touch() override;

        bool isDeclOnly() const;
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

        tempo_utils::Result<ActionSymbol *> declareAction(
            const std::string &name,
            lyric_object::AccessType access);

        tempo_utils::Status prepareAction(
            const std::string &name,
            const lyric_common::TypeDef &receiverType,
            CallableInvoker &invoker,
            bool isReceiver = false);

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

        tempo_utils::Result<ImplHandle *> declareImpl(const lyric_common::TypeDef &implType);

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
        ObjectState *m_state;

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
