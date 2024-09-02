#ifndef LYRIC_ASSEMBLER_UNDECLARED_SYMBOL_H
#define LYRIC_ASSEMBLER_UNDECLARED_SYMBOL_H

#include "abstract_symbol.h"

namespace lyric_assembler {

    class UndeclaredSymbol : public AbstractSymbol {

    public:
        UndeclaredSymbol(const lyric_common::SymbolUrl &undeclaredUrl, lyric_object::LinkageSection section);

        bool isImported() const override;
        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getTypeDef() const override;

        lyric_object::LinkageSection getLinkage() const;

    private:
        lyric_common::SymbolUrl m_undeclaredUrl;
        lyric_object::LinkageSection m_section;
    };
}

#endif // LYRIC_ASSEMBLER_UNDECLARED_SYMBOL_H
