#ifndef LYRIC_TYPING_SUMMON_REIFIER_H
#define LYRIC_TYPING_SUMMON_REIFIER_H

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

    class SummonReifier {

    public:
        explicit SummonReifier(lyric_assembler::ObjectState *state);
        explicit SummonReifier(TypeSystem *typeSystem);

        tempo_utils::Status initialize(lyric_assembler::ActionSymbol *actionSymbol);
        tempo_utils::Status reifyNextArgument(const lyric_common::TypeDef &argumentType);
        tempo_utils::Status finalize();

        lyric_common::TypeDef getReifiedArgument(int index) const;
        int numReifiedArguments() const;
        tempo_utils::Result<lyric_common::TypeDef> reifySummonType() const;
        tempo_utils::Result<Option<tu_uint16>> findFirstPlacement(const lyric_common::TypeDef &argumentType) const;

    private:
        std::unique_ptr<internal::DispatchState> m_state;
        lyric_assembler::ActionSymbol *m_actionSymbol = nullptr;
        lyric_common::SymbolUrl m_conceptUrl;
        std::vector<lyric_assembler::Parameter> m_unifiedParameters;
        Option<lyric_assembler::Parameter> m_restParameter;
        bool m_initialized = false;

        std::vector<lyric_common::TypeDef> m_argumentTypes;
        lyric_common::TypeDef m_summonType;
        bool m_finalized = false;

        tempo_utils::Status initialize(
            const lyric_assembler::AbstractPlacement *placement,
            const std::vector<lyric_common::TypeDef> &callsiteArguments = {});
    };
}

#endif // LYRIC_TYPING_SUMMON_REIFIER_H
