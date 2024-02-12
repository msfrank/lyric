#ifndef LYRIC_ASSEMBLER_EXISTENTIAL_SYMBOL_H
#define LYRIC_ASSEMBLER_EXISTENTIAL_SYMBOL_H

#include "abstract_symbol.h"
#include "assembly_state.h"
#include "base_symbol.h"
#include "template_handle.h"
#include "type_handle.h"

namespace lyric_assembler {

    struct ExistentialSymbolPriv {
        lyric_object::AccessType access;
        lyric_object::DeriveType derive;
        TypeHandle *existentialType;
        TemplateHandle *existentialTemplate;
        ExistentialSymbol *superExistential;
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
            BlockHandle *parentBlock,
            AssemblyState *state);
        ExistentialSymbol(
            const lyric_common::SymbolUrl &existentialUrl,
            lyric_object::AccessType access,
            lyric_object::DeriveType derive,
            ExistentialAddress address,
            TypeHandle *existentialType,
            TemplateHandle *existentialTemplate,
            ExistentialSymbol *superExistential,
            BlockHandle *parentBlock,
            AssemblyState *state);
        ExistentialSymbol(
            const lyric_common::SymbolUrl &existentialUrl,
            lyric_importer::ExistentialImport *existentialImport,
            AssemblyState *state);

        lyric_object::LinkageSection getLinkage() const override;

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getAssignableType() const override;
        TypeSignature getTypeSignature() const override;
        void touch() override;

        lyric_object::DeriveType getDeriveType() const;

        ExistentialSymbol *superExistential() const;
        TypeHandle *existentialType() const;
        TemplateHandle *existentialTemplate() const;

        /*
         * subtype tracking for sealed class
         */
        bool hasSealedType(const lyric_common::TypeDef &sealedType) const;
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesBegin() const;
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesEnd() const;
        tempo_utils::Status putSealedType(const lyric_common::TypeDef &sealedType);

    private:
        lyric_common::SymbolUrl m_existentialUrl;
        lyric_importer::ExistentialImport *m_existentialImport;
        AssemblyState *m_state;

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
