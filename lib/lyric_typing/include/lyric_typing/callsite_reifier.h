#ifndef LYRIC_TYPING_CALLSITE_REIFIER_H
#define LYRIC_TYPING_CALLSITE_REIFIER_H

#include <lyric_common/symbol_url.h>

#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/assembler_result.h>
#include <lyric_assembler/assembler_types.h>
#include <lyric_assembler/object_state.h>
#include <lyric_assembler/callable_invoker.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/constructable_invoker.h>

#include "type_system.h"

namespace lyric_typing {

    class CallsiteReifier : public lyric_assembler::AbstractCallsiteReifier {

    public:
        explicit CallsiteReifier(TypeSystem *typeSystem);

        bool isValid() const;

        tempo_utils::Status initialize(
            const lyric_assembler::ConstructableInvoker &invoker,
            const std::vector<lyric_common::TypeDef> &callsiteArguments = {});
        tempo_utils::Status initialize(
            const lyric_assembler::CallableInvoker &invoker,
            const std::vector<lyric_common::TypeDef> &callsiteArguments = {});

        std::vector<lyric_common::TypeDef> getCallsiteArguments() const;

        size_t numReifiedArguments() const override;
        tempo_utils::Status reifyNextArgument(const lyric_common::TypeDef &argumentType) override;
        tempo_utils::Result<lyric_common::TypeDef> reifyNextContext() override;
        tempo_utils::Result<lyric_common::TypeDef> reifyResult(
            const lyric_common::TypeDef &returnType) const override;

    private:
        TypeSystem *m_typeSystem;

        lyric_assembler::TemplateHandle *m_invokerTemplate;
        std::vector<lyric_assembler::Parameter> m_unifiedParameters;
        Option<lyric_assembler::Parameter> m_restParameter;
        std::vector<lyric_common::TypeDef> m_callsiteArguments;
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
