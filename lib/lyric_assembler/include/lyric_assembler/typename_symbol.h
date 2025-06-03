#ifndef LYRIC_ASSEMBLER_TYPE_NAME_H
#define LYRIC_ASSEMBLER_TYPE_NAME_H

#include "abstract_symbol.h"
#include "assembler_types.h"

namespace lyric_assembler {

    class TypenameSymbol : public AbstractSymbol {

    public:
        explicit TypenameSymbol(const lyric_common::SymbolUrl &typenameUrl);

        bool isImported() const override;
        bool isCopied() const override;
        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getTypeDef() const override;

    private:
        lyric_common::SymbolUrl m_typenameUrl;
    };

    inline const TypenameSymbol *cast_symbol_to_typename(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::TYPENAME);
        return static_cast<const TypenameSymbol *>(sym);   // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    inline TypenameSymbol *cast_symbol_to_typename(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::TYPENAME);
        return static_cast<TypenameSymbol *>(sym);         // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_TYPE_NAME_H