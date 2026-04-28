#ifndef LYRIC_ASSEMBLER_ARGUMENT_VARIABLE_H
#define LYRIC_ASSEMBLER_ARGUMENT_VARIABLE_H

#include "abstract_symbol.h"
#include "assembler_types.h"
#include "object_state.h"

namespace lyric_assembler {

    class ArgumentVariable : public AbstractSymbol {
    public:
        ArgumentVariable(
            const lyric_common::SymbolUrl &argumentUrl,
            const lyric_common::TypeDef &assignableType,
            BindingType bindingType,
            ArgumentOffset offset,
            ObjectState *state);
        ArgumentVariable(
            const lyric_common::SymbolUrl &argumentUrl,
            ArgumentOffset offset,
            ObjectState *state);

        bool isImported() const override;
        bool isCopied() const override;
        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getTypeDef() const override;
        BlockHandle *definitionBlock() override;

        std::string getName() const;
        BindingType getBindingType() const;
        ArgumentOffset getOffset() const;

    private:
        lyric_common::SymbolUrl m_argumentUrl;
        lyric_common::TypeDef m_assignableType;
        BindingType m_bindingType;
        ArgumentOffset m_offset;
        ObjectState *m_state;
    };

    inline const ArgumentVariable *cast_symbol_to_argument(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::ARGUMENT);
        return static_cast<const ArgumentVariable *>(sym);      // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    inline ArgumentVariable *cast_symbol_to_argument(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::ARGUMENT);
        return static_cast<ArgumentVariable *>(sym);            // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_ARGUMENT_VARIABLE_H
