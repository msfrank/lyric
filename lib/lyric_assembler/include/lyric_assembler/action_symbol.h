#ifndef LYRIC_ASSEMBLER_ACTION_SYMBOL_H
#define LYRIC_ASSEMBLER_ACTION_SYMBOL_H

#include "abstract_resolver.h"
#include "abstract_symbol.h"
#include "base_symbol.h"
#include "object_state.h"

namespace lyric_assembler {

    struct ActionSymbolPriv {
        std::vector<Parameter> listParameters;
        std::vector<Parameter> namedParameters;
        Option<Parameter> restParameter;
        lyric_common::TypeDef returnType;
        lyric_common::SymbolUrl receiverUrl;
        bool isHidden = false;
        TemplateHandle *actionTemplate = nullptr;
        bool isDeclOnly = false;
        BlockHandle *parentBlock = nullptr;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> initializers;
    };

    class ActionSymbol : public BaseSymbol<ActionSymbolPriv> {
    public:
        ActionSymbol(
            const lyric_common::SymbolUrl &actionUrl,
            const lyric_common::SymbolUrl &receiverUrl,
            bool isHidden,
            TemplateHandle *actionTemplate,
            bool isDeclOnly,
            BlockHandle *parentBlock,
            ObjectState *state);

        ActionSymbol(
            const lyric_common::SymbolUrl &actionUrl,
            const lyric_common::SymbolUrl &receiverUrl,
            bool isHidden,
            bool isDeclOnly,
            BlockHandle *parentBlock,
            ObjectState *state);

        ActionSymbol(
            const lyric_common::SymbolUrl &actionUrl,
            lyric_importer::ActionImport *actionImport,
            bool isCopied,
            ObjectState *state);

        lyric_object::LinkageSection getLinkage() const override;

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getTypeDef() const override;

        bool isDeclOnly() const;

        tempo_utils::Status defineAction(
            const ParameterPack &parameterPack,
            const lyric_common::TypeDef &returnType);

        lyric_common::TypeDef getReturnType() const;
        lyric_common::SymbolUrl getReceiverUrl() const;
        bool isHidden() const;

        AbstractResolver *actionResolver() const;
        TemplateHandle *actionTemplate() const;

        std::vector<Parameter>::const_iterator listPlacementBegin() const;
        std::vector<Parameter>::const_iterator listPlacementEnd() const;
        std::vector<Parameter>::const_iterator namedPlacementBegin() const;
        std::vector<Parameter>::const_iterator namedPlacementEnd() const;
        const Parameter *restPlacement() const;

        bool hasInitializer(const std::string &name) const;
        lyric_common::SymbolUrl getInitializer(const std::string &name) const;
        tempo_utils::Status putInitializer(const std::string &name, const lyric_common::SymbolUrl &initializer);

    private:
        lyric_common::SymbolUrl m_actionUrl;
        lyric_importer::ActionImport *m_actionImport = nullptr;
        ObjectState *m_state;

        ActionSymbolPriv *load() override;
    };

    inline const ActionSymbol *cast_symbol_to_action(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::ACTION);
        return static_cast<const ActionSymbol *>(sym);      // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    inline ActionSymbol *cast_symbol_to_action(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::ACTION);
        return static_cast<ActionSymbol *>(sym);            // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_ACTION_SYMBOL_H
