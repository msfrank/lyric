#ifndef LYRIC_ASSEMBLER_EXTENSION_CALLABLE_H
#define LYRIC_ASSEMBLER_EXTENSION_CALLABLE_H

#include <lyric_common/symbol_url.h>

#include "abstract_callsite_reifier.h"
#include "abstract_callable.h"
#include "assembler_types.h"

namespace lyric_assembler {

    // forward declarations
    class CallSymbol;
    class ProcHandle;
    class TemplateHandle;

    class ExtensionCallable : public AbstractCallable {

        enum class InvokeType {
            INVALID,
            INLINE,
            VIRTUAL,
        };

    public:
        ExtensionCallable();
        ExtensionCallable(CallSymbol *callSymbol, ProcHandle *procHandle);
        ExtensionCallable(CallSymbol *callSymbol, const DataReference &implRef);
//        ExtensionInvoker(
//            ConceptSymbol *conceptSymbol,
//            ActionSymbol *actionSymbol,
//            const DataReference &ref);

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
        InvokeType m_type;
        CallSymbol *m_call;
        ProcHandle *m_proc;
        //ConceptSymbol *m_concept;
        //ActionSymbol *m_action;
        DataReference m_implRef;

        std::vector<lyric_object::Parameter> m_parameters;
        Option<lyric_object::Parameter> m_rest;
        std::vector<lyric_object::TemplateParameter> m_templateParameters;
        lyric_common::SymbolUrl m_templateUrl;

        void checkValid() const;
    };
}

#endif // LYRIC_ASSEMBLER_EXTENSION_CALLABLE_H
