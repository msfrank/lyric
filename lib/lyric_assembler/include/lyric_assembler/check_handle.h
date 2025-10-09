#ifndef LYRIC_ASSEMBLER_CHECK_HANDLE_H
#define LYRIC_ASSEMBLER_CHECK_HANDLE_H

#include "assembler_types.h"
#include "catch_handle.h"
#include "code_fragment.h"
#include "object_state.h"

namespace lyric_assembler {

    class CheckHandle {
    public:
        CheckHandle(
            const JumpLabel &startInclusive,
            const DataReference &caughtRef,
            ProcHandle *procHandle,
            ObjectState *state);

        JumpLabel getStartInclusive() const;
        JumpLabel getEndExclusive() const;
        DataReference getCaughtReference() const;

        tempo_utils::Result<CatchHandle *> declareCatch(const JumpLabel &startInclusive);
        std::vector<CatchHandle *>::const_iterator catchesBegin() const;
        std::vector<CatchHandle *>::const_iterator catchesEnd() const;
        int numCatches() const;

        void appendChild(CheckHandle *child);
        std::vector<CheckHandle *>::const_iterator childrenBegin() const;
        std::vector<CheckHandle *>::const_iterator childrenEnd() const;
        int numChildren() const;

        tempo_utils::Status finalizeCheck(const JumpLabel &endExclusive);

    private:
        JumpLabel m_startInclusive;
        JumpLabel m_endExclusive;
        DataReference m_caughtRef;
        ProcHandle *m_procHandle;
        std::vector<CatchHandle *> m_catches;
        std::vector<CheckHandle *> m_children;
        ObjectState *m_state;
    };
}

#endif // LYRIC_ASSEMBLER_CHECK_HANDLE_H