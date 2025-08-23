#ifndef LYRIC_ASSEMBLER_FIELD_SYMBOL_H
#define LYRIC_ASSEMBLER_FIELD_SYMBOL_H

#include "abstract_symbol.h"
#include "base_symbol.h"
#include "initializer_handle.h"
#include "object_state.h"
#include "type_handle.h"

namespace lyric_assembler {

    struct FieldSymbolPriv {
        bool isHidden;
        bool isVariable;
        bool isDeclOnly;
        std::unique_ptr<InitializerHandle> initializerHandle;
        BlockHandle *parentBlock;
        TypeHandle *fieldType;
    };

    class FieldSymbol : public BaseSymbol<FieldSymbolPriv> {
    public:
        FieldSymbol(
            const lyric_common::SymbolUrl &fieldUrl,
            bool isHidden,
            bool isVariable,
            TypeHandle *fieldType,
            bool isDeclOnly,
            BlockHandle *parentBlock,
            ObjectState *state);
        FieldSymbol(
            const lyric_common::SymbolUrl &fieldUrl,
            lyric_importer::FieldImport *fieldImport,
            bool isCopied,
            ObjectState *state);

        lyric_object::LinkageSection getLinkage() const override;

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getTypeDef() const override;

        std::string getName() const;
        bool isHidden() const;
        bool isVariable() const;
        bool isDeclOnly() const;

        bool hasInitializer() const;
        lyric_common::SymbolUrl getInitializer() const;
        tempo_utils::Result<InitializerHandle *> defineInitializer();

        DataReference getReference() const;

    private:
        lyric_common::SymbolUrl m_fieldUrl;
        lyric_importer::FieldImport * m_fieldImport = nullptr;
        ObjectState *m_state;

        FieldSymbolPriv *load() override;
    };

    inline const FieldSymbol *cast_symbol_to_field(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::FIELD);
        return static_cast<const FieldSymbol *>(sym);       // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    inline FieldSymbol *cast_symbol_to_field(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::FIELD);
        return static_cast<FieldSymbol *>(sym);             // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_FIELD_SYMBOL_H
