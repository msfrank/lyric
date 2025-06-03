#ifndef LYRIC_ASSEMBLER_LINKAGE_SYMBOL_H
#define LYRIC_ASSEMBLER_LINKAGE_SYMBOL_H

#include <lyric_object/object_types.h>

#include "abstract_symbol.h"

namespace lyric_assembler {

    class LinkageSymbol : public AbstractSymbol {

    public:
        LinkageSymbol(const lyric_common::SymbolUrl &linkageUrl, lyric_object::LinkageSection linkageSection);

        bool isImported() const override;
        bool isCopied() const override;
        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getTypeDef() const override;

        lyric_object::LinkageSection getLinkage() const;

    private:
        lyric_common::SymbolUrl m_linkageUrl;
        lyric_object::LinkageSection m_linkageSection;
    };

    static inline const LinkageSymbol *cast_symbol_to_linkage(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::LINKAGE);
        return static_cast<const LinkageSymbol *>(sym);    // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    static inline LinkageSymbol *cast_symbol_to_linkage(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::LINKAGE);
        return static_cast<LinkageSymbol *>(sym);          // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_LINKAGE_SYMBOL_H
