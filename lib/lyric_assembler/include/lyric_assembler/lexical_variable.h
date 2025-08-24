#ifndef LYRIC_ASSEMBLER_LEXICAL_VARIABLE_H
#define LYRIC_ASSEMBLER_LEXICAL_VARIABLE_H

#include "abstract_symbol.h"
#include "assembler_types.h"

namespace lyric_assembler {

    class LexicalVariable : public AbstractSymbol {

    public:
        LexicalVariable(
            const lyric_common::SymbolUrl &lexicalUrl,
            const lyric_common::TypeDef &assignableType,
            LexicalOffset offset);

        bool isImported() const override;
        bool isCopied() const override;
        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getTypeDef() const override;

        std::string getName() const;
        LexicalOffset getOffset() const;

    private:
        lyric_common::SymbolUrl m_lexicalUrl;
        lyric_common::TypeDef m_assignableType;
        LexicalOffset m_offset;
    };

    inline const LexicalVariable *cast_symbol_to_lexical(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::LEXICAL);
        return static_cast<const LexicalVariable *>(sym);   // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    inline LexicalVariable *cast_symbol_to_lexical(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::LEXICAL);
        return static_cast<LexicalVariable *>(sym);         // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_LEXICAL_VARIABLE_H
