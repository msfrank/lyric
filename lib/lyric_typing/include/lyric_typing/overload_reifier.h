#ifndef LYRIC_TYPING_OVERLOAD_REIFIER_H
#define LYRIC_TYPING_OVERLOAD_REIFIER_H

#include <lyric_assembler/object_state.h>

#include "type_system.h"

namespace lyric_typing {

    // forward declarations
    namespace internal {
        struct DispatchState;
    }

    class OverloadReifier {
    public:
        explicit OverloadReifier(lyric_assembler::ObjectState *state);
        explicit OverloadReifier(TypeSystem *typeSystem);
        ~OverloadReifier();

        bool isValid() const;

        tempo_utils::Status initialize(
            lyric_assembler::ActionSymbol *actionSymbol,
            const std::vector<lyric_common::TypeDef> &overloadArguments = {});
        tempo_utils::Status initialize(
            lyric_assembler::CallSymbol *callSymbol,
            const std::vector<lyric_common::TypeDef> &overloadArguments = {});

        std::vector<lyric_common::TypeDef> getOverloadArguments() const;

        tempo_utils::Result<lyric_assembler::ParameterPack> reifyParameters(
            const lyric_assembler::ParameterPack &parameterPack);

        tempo_utils::Result<lyric_common::TypeDef> reifyResult(const lyric_common::TypeDef &resultType);

    private:
        std::unique_ptr<internal::DispatchState> m_state;
        std::vector<lyric_assembler::Parameter> m_unifiedParameters;
        Option<lyric_assembler::Parameter> m_restParameter;
        lyric_common::TypeDef m_returnType;
        std::vector<lyric_common::TypeDef> m_overloadArguments;
        lyric_common::TypeDef m_resultType;
        bool m_initialized;
    };
}
#endif // LYRIC_TYPING_OVERLOAD_REIFIER_H
