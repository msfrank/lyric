#ifndef LYRIC_ASSEMBLER_ACTION_CALLABLE_H
#define LYRIC_ASSEMBLER_ACTION_CALLABLE_H

#include "abstract_callsite_reifier.h"
#include "abstract_callable.h"
#include "assembler_types.h"

namespace lyric_assembler {

    // forward declarations
    class ActionSymbol;
    class BlockHandle;
    class ConceptSymbol;

    class ActionCallable : public AbstractCallable {

    public:
        ActionCallable();
        ActionCallable(ActionSymbol *action, const ConceptAddress &address);

        bool isValid() const;

        lyric_assembler::TemplateHandle *getTemplate() const override;
        std::vector<Parameter>::const_iterator listPlacementBegin() const override;
        std::vector<Parameter>::const_iterator listPlacementEnd() const override;
        std::vector<Parameter>::const_iterator namedPlacementBegin() const override;
        std::vector<Parameter>::const_iterator namedPlacementEnd() const override;
        const Parameter *restPlacement() const override;
        bool hasInitializer(const std::string &name) const override;
        lyric_common::SymbolUrl getInitializer(const std::string &name) const override;

        tempo_utils::Result<lyric_common::TypeDef> invoke(
            BlockHandle *block,
            const AbstractCallsiteReifier &reifier) override;

    private:
        ActionSymbol *m_action;
        ConceptAddress m_address;
//
//        std::vector<lyric_object::Parameter> m_parameters;
//        Option<lyric_object::Parameter> m_rest;
//        std::vector<lyric_object::TemplateParameter> m_templateParameters;
//        lyric_common::SymbolUrl m_templateUrl;

        void checkValid() const;
    };
}

#endif // LYRIC_ASSEMBLER_ACTION_CALLABLE_H
