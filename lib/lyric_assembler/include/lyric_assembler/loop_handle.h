#ifndef LYRIC_ASSEMBLER_LOOP_HANDLE_H
#define LYRIC_ASSEMBLER_LOOP_HANDLE_H

#include "assembler_types.h"
#include "catch_handle.h"
#include "code_fragment.h"
#include "object_state.h"

namespace lyric_assembler {

    class LoopHandle {
    public:
        LoopHandle(
            const JumpLabel &topOfLoop,
            ProcHandle *procHandle,
            ObjectState *state);

        JumpLabel getTopOfLoop() const;

        tempo_utils::Status continueLoop(const JumpTarget &continueTarget);
        std::vector<JumpTarget>::const_iterator continueTargetsBegin() const;
        std::vector<JumpTarget>::const_iterator continueTargetsEnd() const;

        tempo_utils::Status breakLoop(const JumpTarget &breakTarget);
        std::vector<JumpTarget>::const_iterator breakTargetsBegin() const;
        std::vector<JumpTarget>::const_iterator breakTargetsEnd() const;

        tempo_utils::Status finalizeLoop(const JumpLabel &loopExit);

    private:
        JumpLabel m_topOfLoop;
        std::vector<JumpTarget> m_continueTargets;
        std::vector<JumpTarget> m_breakTargets;
        JumpLabel m_loopExit;
        ProcHandle *m_procHandle;
        ObjectState *m_state;
    };
}

#endif // LYRIC_ASSEMBLER_LOOP_HANDLE_H
