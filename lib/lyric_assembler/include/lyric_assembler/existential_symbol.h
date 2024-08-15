#ifndef LYRIC_ASSEMBLER_EXISTENTIAL_SYMBOL_H
#define LYRIC_ASSEMBLER_EXISTENTIAL_SYMBOL_H

#include <lyric_importer/existential_import.h>

#include "abstract_symbol.h"
#include "object_state.h"
#include "base_symbol.h"
#include "impl_handle.h"
#include "template_handle.h"
#include "type_handle.h"

namespace lyric_assembler {

    struct ExistentialSymbolPriv {
        lyric_object::AccessType access;
        lyric_object::DeriveType derive;
        bool isDeclOnly;
        TypeHandle *existentialType;
        TemplateHandle *existentialTemplate;
        ExistentialSymbol *superExistential;
        absl::flat_hash_map<std::string, BoundMethod> methods;
        absl::flat_hash_map<lyric_common::SymbolUrl, ImplHandle *> impls;
        absl::flat_hash_set<lyric_common::TypeDef> sealedTypes;
        std::unique_ptr<BlockHandle> existentialBlock;
    };

    class ExistentialSymbol : public BaseSymbol<ExistentialAddress,ExistentialSymbolPriv> {

    public:
        ExistentialSymbol(
            const lyric_common::SymbolUrl &existentialUrl,
            lyric_object::AccessType access,
            lyric_object::DeriveType derive,
            ExistentialAddress address,
            TypeHandle *existentialType,
            ExistentialSymbol *superExistential,
            bool isDeclOnly,
            BlockHandle *parentBlock,
            ObjectState *state);
        ExistentialSymbol(
            const lyric_common::SymbolUrl &existentialUrl,
            lyric_object::AccessType access,
            lyric_object::DeriveType derive,
            ExistentialAddress address,
            TypeHandle *existentialType,
            TemplateHandle *existentialTemplate,
            ExistentialSymbol *superExistential,
            bool isDeclOnly,
            BlockHandle *parentBlock,
            ObjectState *state);
        ExistentialSymbol(
            const lyric_common::SymbolUrl &existentialUrl,
            lyric_importer::ExistentialImport *existentialImport,
            ObjectState *state);

        lyric_object::LinkageSection getLinkage() const override;

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getAssignableType() const override;
        TypeSignature getTypeSignature() const override;
        void touch() override;

        bool isDeclOnly() const;
        lyric_object::DeriveType getDeriveType() const;

        ExistentialSymbol *superExistential() const;
        TypeHandle *existentialType() const;
        TemplateHandle *existentialTemplate() const;

        /*
         * existential method management
         */
        bool hasMethod(const std::string &name) const;
        Option<BoundMethod> getMethod(const std::string &name) const;
        absl::flat_hash_map<std::string, BoundMethod>::const_iterator methodsBegin() const;
        absl::flat_hash_map<std::string, BoundMethod>::const_iterator methodsEnd() const;
        tu_uint32 numMethods() const;

        tempo_utils::Result<lyric_assembler::CallSymbol *> declareMethod(
            const std::string &name,
            lyric_object::AccessType access);

        tempo_utils::Status prepareMethod(
            const std::string &name,
            const lyric_common::TypeDef &receiverType,
            CallableInvoker &invoker,
            bool isReceiver = false);

        /*
         * existential impl management
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
         * subtype tracking for sealed existential
         */
        bool hasSealedType(const lyric_common::TypeDef &sealedType) const;
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesBegin() const;
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesEnd() const;
        tempo_utils::Status putSealedType(const lyric_common::TypeDef &sealedType);

    private:
        lyric_common::SymbolUrl m_existentialUrl;
        lyric_importer::ExistentialImport *m_existentialImport;
        ObjectState *m_state;

        ExistentialSymbolPriv *load() override;
    };

    static inline const ExistentialSymbol *cast_symbol_to_existential(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::EXISTENTIAL);
        return static_cast<const ExistentialSymbol *>(sym);   // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    static inline ExistentialSymbol *cast_symbol_to_existential(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::EXISTENTIAL);
        return static_cast<ExistentialSymbol *>(sym);         // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_EXISTENTIAL_SYMBOL_H
