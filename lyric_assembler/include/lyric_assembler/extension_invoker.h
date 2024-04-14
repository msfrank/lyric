#ifndef LYRIC_ASSEMBLER_EXTENSION_INVOKER_H
#define LYRIC_ASSEMBLER_EXTENSION_INVOKER_H

#include <lyric_common/symbol_url.h>

#include "abstract_callsite_reifier.h"
#include "abstract_invoker.h"
#include "action_symbol.h"
#include "assembler_result.h"
#include "code_builder.h"
#include "concept_symbol.h"
#include "assembler_types.h"

namespace lyric_assembler {

    // forward declarations
    class BlockHandle;

    class CallSymbol;

    class ExtensionInvoker : public AbstractInvoker {

        enum class InvokeType {
            INVALID,
            INLINE,
            VIRTUAL,
        };

    public:
        ExtensionInvoker();
        ExtensionInvoker(CallSymbol *callSymbol, ProcHandle *procHandle);
        ExtensionInvoker(
            ConceptSymbol *conceptSymbol,
            ActionSymbol *actionSymbol,
            const DataReference &ref);

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
        InvokeType m_type;
        CallSymbol *m_call;
        ProcHandle *m_proc;
        ConceptSymbol *m_concept;
        ActionSymbol *m_action;
        DataReference m_ref;

        std::vector<lyric_object::Parameter> m_parameters;
        Option<lyric_object::Parameter> m_rest;
        std::vector<lyric_object::TemplateParameter> m_templateParameters;
        lyric_common::SymbolUrl m_templateUrl;
    };
}

#endif // LYRIC_ASSEMBLER_EXTENSION_INVOKER_H
