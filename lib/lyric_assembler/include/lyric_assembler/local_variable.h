#ifndef LYRIC_ASSEMBLER_LOCAL_VARIABLE_H
#define LYRIC_ASSEMBLER_LOCAL_VARIABLE_H

#include "abstract_symbol.h"
#include "assembler_types.h"
#include "object_state.h"

namespace lyric_assembler {

    class LocalVariable : public AbstractSymbol {

    public:
        LocalVariable(
            const lyric_common::SymbolUrl &localUrl,
            bool isHidden,
            const lyric_common::TypeDef &assignableType,
            tu_uint32 offset,
            ObjectState *state);
        LocalVariable(
            const lyric_common::SymbolUrl &localUrl,
            bool isHidden,
            tu_uint32 offset,
            ObjectState *state);

        bool isImported() const override;
        bool isCopied() const override;
        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getTypeDef() const override;
        BlockHandle *derefBlock() override;

        std::string getName() const;
        bool isHidden() const;
        tu_uint32 getOffset() const;

    private:
        lyric_common::SymbolUrl m_localUrl;
        bool m_isHidden;
        lyric_common::TypeDef m_assignableType;
        tu_uint32 m_offset;
        ObjectState *m_state;
    };

    inline const LocalVariable *cast_symbol_to_local(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::LOCAL);
        return static_cast<const LocalVariable *>(sym);   // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    inline LocalVariable *cast_symbol_to_local(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::LOCAL);
        return static_cast<LocalVariable *>(sym);         // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_LOCAL_VARIABLE_H
