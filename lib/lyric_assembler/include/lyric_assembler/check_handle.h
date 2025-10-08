#ifndef LYRIC_ASSEMBLER_CHECK_HANDLE_H
#define LYRIC_ASSEMBLER_CHECK_HANDLE_H

#include "assembler_types.h"
#include "catch_handle.h"
#include "code_fragment.h"
#include "object_state.h"

namespace lyric_assembler {

    class CheckHandle {
    public:
        CheckHandle(const JumpLabel &startInclusive, ProcHandle *procHandle, ObjectState *state);

        JumpLabel getStartInclusive() const;
        JumpLabel getEndexclusive() const;

        tempo_utils::Result<CatchHandle *> declareException(const lyric_common::TypeDef &exceptionType);
        absl::flat_hash_map<lyric_common::TypeDef,CatchHandle *>::const_iterator exceptionsBegin() const;
        absl::flat_hash_map<lyric_common::TypeDef,CatchHandle *>::const_iterator exceptionsEnd() const;
        int numExceptions() const;

        tempo_utils::Status finalizeCheck(const JumpLabel &endExclusive);

    private:
        JumpLabel m_startInclusive;
        JumpLabel m_endExclusive;
        ProcHandle *m_procHandle;
        absl::flat_hash_map<lyric_common::TypeDef,CatchHandle *> m_exceptions;
        ObjectState *m_state;
    };
}

#endif // LYRIC_ASSEMBLER_CHECK_HANDLE_H