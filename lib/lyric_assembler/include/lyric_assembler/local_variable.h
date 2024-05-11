#ifndef LYRIC_ASSEMBLER_LOCAL_VARIABLE_H
#define LYRIC_ASSEMBLER_LOCAL_VARIABLE_H

#include "abstract_symbol.h"
#include "assembler_types.h"

namespace lyric_assembler {

    class LocalVariable : public AbstractSymbol {

    public:
        LocalVariable(
            const lyric_common::SymbolUrl &localUrl,
            const lyric_common::TypeDef &assignableType,
            LocalOffset offset);

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getAssignableType() const override;
        TypeSignature getTypeSignature() const override;
        void touch() override;

        std::string getName() const;
        LocalOffset getOffset() const;

    private:
        lyric_common::SymbolUrl m_localUrl;
        lyric_common::TypeDef m_assignableType;
        LocalOffset m_offset;
    };

    static inline const LocalVariable *cast_symbol_to_local(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::LOCAL);
        return static_cast<const LocalVariable *>(sym);   // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    static inline LocalVariable *cast_symbol_to_local(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::LOCAL);
        return static_cast<LocalVariable *>(sym);         // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_LOCAL_VARIABLE_H
