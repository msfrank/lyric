#ifndef LYRIC_ASSEMBLER_ACTION_SYMBOL_H
#define LYRIC_ASSEMBLER_ACTION_SYMBOL_H

#include "abstract_symbol.h"
#include "base_symbol.h"
#include "assembly_state.h"

namespace lyric_assembler {

    struct ActionSymbolPriv {
        std::vector<lyric_object::Parameter> parameters;
        Option<lyric_object::Parameter> rest;
        lyric_common::TypeDef returnType;
        lyric_common::SymbolUrl receiverUrl;
        TemplateHandle *actionTemplate = nullptr;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> initializers;
    };

    class ActionSymbol : public BaseSymbol<ActionAddress,ActionSymbolPriv> {
    public:
        ActionSymbol(
            const lyric_common::SymbolUrl &actionUrl,
            const std::vector<lyric_object::Parameter> &parameters,
            const Option<lyric_object::Parameter> &rest,
            const lyric_common::TypeDef &returnType,
            const lyric_common::SymbolUrl &receiverUrl,
            ActionAddress address,
            TemplateHandle *actionTemplate,
            AssemblyState *state);

        ActionSymbol(
            const lyric_common::SymbolUrl &actionUrl,
            const std::vector<lyric_object::Parameter> &parameters,
            const Option<lyric_object::Parameter> &rest,
            const lyric_common::TypeDef &returnType,
            const lyric_common::SymbolUrl &receiverUrl,
            ActionAddress address,
            AssemblyState *state);

        ActionSymbol(
            const lyric_common::SymbolUrl &actionUrl,
            lyric_importer::ActionImport *actionImport,
            AssemblyState *state);

        lyric_object::LinkageSection getLinkage() const override;

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getAssignableType() const override;
        TypeSignature getTypeSignature() const override;
        void touch() override;

        std::vector<lyric_object::Parameter> getParameters() const;
        Option<lyric_object::Parameter> getRest() const;
        lyric_common::TypeDef getReturnType() const;
        lyric_common::SymbolUrl getReceiverUrl() const;

        TemplateHandle *actionTemplate();

        std::vector<lyric_object::Parameter>::const_iterator placementBegin() const;
        std::vector<lyric_object::Parameter>::const_iterator placementEnd() const;

        bool hasInitializer(const std::string &name) const;
        lyric_common::SymbolUrl getInitializer(const std::string &name) const;
        void putInitializer(const std::string &name, const lyric_common::SymbolUrl &initializer);

    private:
        lyric_common::SymbolUrl m_actionUrl;
        lyric_importer::ActionImport *m_actionImport = nullptr;
        AssemblyState *m_state;

        ActionSymbolPriv *load() override;
    };

    static inline const ActionSymbol *cast_symbol_to_action(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::ACTION);
        return static_cast<const ActionSymbol *>(sym);      // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    static inline ActionSymbol *cast_symbol_to_action(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::ACTION);
        return static_cast<ActionSymbol *>(sym);            // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_ACTION_SYMBOL_H
