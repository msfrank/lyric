#ifndef LYRIC_ASSEMBLER_ARGUMENT_VARIABLE_H
#define LYRIC_ASSEMBLER_ARGUMENT_VARIABLE_H

#include "abstract_symbol.h"
#include "assembler_types.h"

namespace lyric_assembler {

    class ArgumentVariable : public AbstractSymbol {
    public:
        ArgumentVariable(
            const lyric_common::SymbolUrl &argumentUrl,
            const lyric_common::TypeDef &assignableType,
            lyric_parser::BindingType binding,
            ArgumentOffset offset);

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getAssignableType() const override;
        TypeSignature getTypeSignature() const override;
        void touch() override;

        std::string getName() const;
        lyric_parser::BindingType getBindingType() const;
        ArgumentOffset getOffset() const;

    private:
        lyric_common::SymbolUrl m_argumentUrl;
        lyric_common::TypeDef m_assignableType;
        lyric_parser::BindingType m_binding;
        ArgumentOffset m_offset;
    };

    static inline const ArgumentVariable *cast_symbol_to_argument(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::ARGUMENT);
        return static_cast<const ArgumentVariable *>(sym);      // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    static inline ArgumentVariable *cast_symbol_to_argument(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::ARGUMENT);
        return static_cast<ArgumentVariable *>(sym);            // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_ARGUMENT_VARIABLE_H
