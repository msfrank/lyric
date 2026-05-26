#ifndef LYRIC_TYPING_CALLSITE_REIFIER_H
#define LYRIC_TYPING_CALLSITE_REIFIER_H

#include <lyric_common/symbol_url.h>

#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/assembler_types.h>
#include <lyric_assembler/object_state.h>
#include <lyric_assembler/call_symbol.h>

#include "type_system.h"

namespace lyric_typing {

    // forward declarations
    namespace internal {
        struct DispatchState;
    }

    class CallsiteReifier : public lyric_assembler::AbstractCallsiteReifier {

    public:
        explicit CallsiteReifier(lyric_assembler::ObjectState *state);
        explicit CallsiteReifier(TypeSystem *typeSystem);
        ~CallsiteReifier() override;

        bool isValid() const;

        tempo_utils::Status initialize(
            const lyric_assembler::AbstractPlacement *placement,
            const std::vector<lyric_common::TypeDef> &callsiteArguments = {});
        tempo_utils::Status initialize(
            const lyric_common::TypeDef &receiverType,
            const lyric_assembler::AbstractPlacement *placement);
        tempo_utils::Status initialize(
            const lyric_common::TypeDef &receiverType,
            const lyric_assembler::AbstractPlacement *placement,
            const std::vector<lyric_common::TypeDef> &callsiteArguments);

        std::vector<lyric_common::TypeDef> getCallsiteArguments() const;

        size_t numReifiedArguments() const override;
        tempo_utils::Status reifyNextArgument(const lyric_common::TypeDef &argumentType) override;
        tempo_utils::Result<lyric_common::TypeDef> reifyNextContext() override;
        tempo_utils::Result<lyric_common::TypeDef> reifyResult(
            const lyric_common::TypeDef &returnType) const override;

    private:
        std::unique_ptr<internal::DispatchState> m_state;
        std::vector<lyric_assembler::Parameter> m_unifiedParameters;
        Option<lyric_assembler::Parameter> m_restParameter;
        std::vector<lyric_common::TypeDef> m_callsiteArguments;
        bool m_initialized;

        std::vector<lyric_common::TypeDef> m_argumentTypes;
        lyric_common::TypeDef m_restType;
    };
}

#endif // LYRIC_TYPING_CALLSITE_REIFIER_H
