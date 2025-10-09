#ifndef LYRIC_ASSEMBLER_CATCH_HANDLE_H
#define LYRIC_ASSEMBLER_CATCH_HANDLE_H

#include "assembler_types.h"
#include "code_fragment.h"
#include "object_state.h"

namespace lyric_assembler {

    class CatchHandle {
    public:
        CatchHandle(const JumpLabel &startInclusive, ObjectState *state);

        JumpLabel getStartInclusive() const;
        JumpLabel getEndExclusive() const;

        lyric_common::TypeDef getExceptionType() const;
        tempo_utils::Status setExceptionType(const lyric_common::TypeDef &exceptionType);

        JumpTarget getResumeTarget() const;
        tempo_utils::Status setResumeTarget(const JumpTarget &resumeTarget);

        ObjectState *objectState() const;

        tempo_utils::Status finalizeCatch(const JumpLabel &endExclusive);

    private:
        JumpLabel m_startInclusive;
        JumpLabel m_endExclusive;
        ObjectState *m_state;
        lyric_common::TypeDef m_exceptionType;
        JumpTarget m_resumeTarget;
    };
}

#endif // LYRIC_ASSEMBLER_CATCH_HANDLE_H