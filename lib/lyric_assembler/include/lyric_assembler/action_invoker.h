#ifndef LYRIC_ASSEMBLER_ACTION_INVOKER_H
#define LYRIC_ASSEMBLER_ACTION_INVOKER_H

#include "abstract_callsite_reifier.h"
#include "abstract_invoker.h"
#include "assembler_types.h"

namespace lyric_assembler {

    // forward declarations
    class ActionSymbol;
    class BlockHandle;
    class ConceptSymbol;

    class ActionInvoker : public AbstractInvoker {

    public:
        ActionInvoker();
        ActionInvoker(ActionSymbol *action, const ConceptAddress &address);

        bool isValid() const;

        std::vector<lyric_object::Parameter> getParameters() const;
        Option<lyric_object::Parameter> getRest() const;
        lyric_common::SymbolUrl getTemplateUrl() const;
        std::vector<lyric_object::TemplateParameter> getTemplateParameters() const;

        std::vector<lyric_object::Parameter>::const_iterator placementBegin() const override;
        std::vector<lyric_object::Parameter>::const_iterator placementEnd() const override;
        bool hasInitializer(const std::string &name) const override;
        lyric_common::SymbolUrl getInitializer(const std::string &name) const override;

        tempo_utils::Result<lyric_common::TypeDef> invoke(BlockHandle *block, const AbstractCallsiteReifier &reifier);

    private:
        ActionSymbol *m_action;
        ConceptAddress m_address;

        std::vector<lyric_object::Parameter> m_parameters;
        Option<lyric_object::Parameter> m_rest;
        std::vector<lyric_object::TemplateParameter> m_templateParameters;
        lyric_common::SymbolUrl m_templateUrl;
    };
}

#endif // LYRIC_ASSEMBLER_ACTION_INVOKER_H
