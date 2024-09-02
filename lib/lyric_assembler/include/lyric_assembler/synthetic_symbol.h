#ifndef LYRIC_ASSEMBLER_SYNTHETIC_SYMBOL_H
#define LYRIC_ASSEMBLER_SYNTHETIC_SYMBOL_H

#include "abstract_symbol.h"

namespace lyric_assembler {

    class SyntheticSymbol : public AbstractSymbol {

    public:
        explicit SyntheticSymbol(
            const lyric_common::SymbolUrl &syntheticUrl,
            SyntheticType syntheticType,
            const lyric_common::TypeDef &assignableType);

        bool isImported() const override;
        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getTypeDef() const override;

        SyntheticType getSyntheticType() const;

    private:
        lyric_common::SymbolUrl m_syntheticUrl;
        SyntheticType m_syntheticType;
        lyric_common::TypeDef m_assignableType;
    };

    static inline const SyntheticSymbol *cast_symbol_to_synthetic(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::SYNTHETIC);
        return static_cast<const SyntheticSymbol *>(sym);   // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    static inline SyntheticSymbol *cast_symbol_to_synthetic(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::SYNTHETIC);
        return static_cast<SyntheticSymbol *>(sym);         // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_SYNTHETIC_SYMBOL_H