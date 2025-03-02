#ifndef LYRIC_ASSEMBLER_BINDING_SYMBOL_H
#define LYRIC_ASSEMBLER_BINDING_SYMBOL_H

#include "abstract_resolver.h"
#include "abstract_symbol.h"
#include "base_symbol.h"
#include "object_state.h"

namespace lyric_assembler {

    struct BindingSymbolPriv {
        lyric_object::AccessType access = lyric_object::AccessType::Invalid;
        TypeHandle *bindingType = nullptr;
        TemplateHandle *bindingTemplate = nullptr;
        TypeHandle *targetType = nullptr;
        BlockHandle *parentBlock = nullptr;
    };

    class BindingSymbol : public BaseSymbol<BindingSymbolPriv> {
    public:
        BindingSymbol(
            const lyric_common::SymbolUrl &bindingUrl,
            lyric_object::AccessType access,
            TypeHandle *bindingType,
            BlockHandle *parentBlock,
            ObjectState *state);
        BindingSymbol(
            const lyric_common::SymbolUrl &bindingUrl,
            lyric_object::AccessType access,
            TypeHandle *bindingType,
            TemplateHandle *bindingTemplate,
            BlockHandle *parentBlock,
            ObjectState *state);

        BindingSymbol(
            const lyric_common::SymbolUrl &bindingUrl,
            lyric_importer::BindingImport *bindingImport,
            bool isCopied,
            ObjectState *state);

        lyric_object::LinkageSection getLinkage() const override;

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getTypeDef() const override;

        std::string getName() const;
        lyric_object::AccessType getAccessType() const;

        TypeHandle *bindingType() const;
        TemplateHandle *bindingTemplate() const;
        TypeHandle *targetType() const;

        AbstractResolver *bindingResolver() const;

        tempo_utils::Status defineTarget(const lyric_common::TypeDef &targetType);

        tempo_utils::Result<lyric_common::TypeDef> resolveTarget(
            const std::vector<lyric_common::TypeDef> &typeArguments);

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
