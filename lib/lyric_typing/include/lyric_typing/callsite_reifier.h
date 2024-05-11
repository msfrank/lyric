#ifndef LYRIC_TYPING_CALLSITE_REIFIER_H
#define LYRIC_TYPING_CALLSITE_REIFIER_H

#include <lyric_common/symbol_url.h>

#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/assembly_state.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/assembler_result.h>
#include <lyric_assembler/assembler_types.h>

#include "type_system.h"

namespace lyric_typing {

    class CallsiteReifier : public lyric_assembler::AbstractCallsiteReifier {

    public:
        CallsiteReifier();
        CallsiteReifier(
            const std::vector<lyric_object::Parameter> &parameters,
            const Option<lyric_object::Parameter> &rest,
            const lyric_common::SymbolUrl &templateUrl,
            const std::vector<lyric_object::TemplateParameter> &templateParameters,
            const std::vector<lyric_common::TypeDef> &callsiteArguments,
            TypeSystem *typeSystem);
        CallsiteReifier(
            const std::vector<lyric_object::Parameter> &parameters,
            const Option<lyric_object::Parameter> &rest,
            TypeSystem *typeSystem);

        bool isValid() const override;

        std::vector<lyric_common::TypeDef> getCallsiteArguments() const;

        tempo_utils::Status initialize();

        lyric_common::TypeDef getArgument(int index) const override;
        std::vector<lyric_common::TypeDef> getArguments() const override;
        int numArguments() const override;

        tempo_utils::Status reifyNextArgument(const lyric_common::TypeDef &argumentType) override;
        tempo_utils::Result<lyric_common::TypeDef> reifyNextContext() override;

        tempo_utils::Result<lyric_common::TypeDef> reifyResult(
            const lyric_common::TypeDef &returnType) const override;

    private:
        std::vector<lyric_object::Parameter> m_parameters;
        Option<lyric_object::Parameter> m_rest;
        lyric_common::SymbolUrl m_templateUrl;
        std::vector<lyric_object::TemplateParameter> m_templateParameters;
        std::vector<lyric_common::TypeDef> m_callsiteArguments;
        TypeSystem *m_typeSystem;

        bool m_initialized;
        std::vector<lyric_common::TypeDef> m_reifiedPlaceholders;
        std::vector<lyric_common::TypeDef> m_argumentTypes;
        lyric_common::TypeDef m_restType;

        tempo_utils::Status checkPlaceholder(
            const lyric_object::TemplateParameter &tp,
            const lyric_common::TypeDef &arg) const;
        tempo_utils::Result<lyric_common::TypeDef> reifySingular(
            const lyric_common::TypeDef &paramType,
            const lyric_common::TypeDef &argType);
        tempo_utils::Result<lyric_common::TypeDef> reifyUnion(
            const lyric_common::TypeDef &paramType,
            const lyric_common::TypeDef &argType);
    };
}

#endif // LYRIC_TYPING_CALLSITE_REIFIER_H
