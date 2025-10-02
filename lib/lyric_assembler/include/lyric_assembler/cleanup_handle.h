#ifndef LYRIC_ASSEMBLER_CLEANUP_HANDLE_H
#define LYRIC_ASSEMBLER_CLEANUP_HANDLE_H

#include "assembler_types.h"
#include "object_state.h"

namespace lyric_assembler {

    class CleanupHandle {
    public:
        CleanupHandle(const JumpLabel &startInclusive, ObjectState *state);

    private:
        JumpLabel m_startInclusive;
        JumpLabel m_endExclusive;
        ObjectState *m_state;
    };
}

#endif // LYRIC_ASSEMBLER_CLEANUP_HANDLE_H