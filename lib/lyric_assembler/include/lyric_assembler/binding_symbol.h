#ifndef LYRIC_ASSEMBLER_BINDING_SYMBOL_H
#define LYRIC_ASSEMBLER_BINDING_SYMBOL_H

#include "abstract_symbol.h"
#include "base_symbol.h"
#include "object_state.h"

namespace lyric_assembler {

    struct BindingSymbolPriv {
        lyric_common::SymbolUrl targetUrl;
        lyric_object::AccessType access;
    };

    class BindingSymbol : public BaseSymbol<BindingSymbolPriv> {
    public:
        BindingSymbol(
            const lyric_common::SymbolUrl &bindingUrl,
            const lyric_common::SymbolUrl &targetUrl,
            lyric_object::AccessType access,
            ObjectState *state);

        BindingSymbol(
            const lyric_common::SymbolUrl &bindingUrl,
            lyric_importer::BindingImport *bindingImport,
            ObjectState *state);

        lyric_object::LinkageSection getLinkage() const override;

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getTypeDef() const override;

        std::string getName() const;
        lyric_object::AccessType getAccessType() const;
        lyric_common::SymbolUrl getTargetUrl() const;

    private:
        lyric_common::SymbolUrl m_bindingUrl;
        lyric_importer::BindingImport *m_bindingImport = nullptr;
        ObjectState *m_state;

        BindingSymbolPriv *load() override;
    };

    static inline const BindingSymbol *cast_symbol_to_binding(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::BINDING);
        return static_cast<const BindingSymbol *>(sym);      // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    static inline BindingSymbol *cast_symbol_to_binding(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::BINDING);
        return static_cast<BindingSymbol *>(sym);            // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_BINDING_SYMBOL_H
