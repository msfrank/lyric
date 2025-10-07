#ifndef LYRIC_ASSEMBLER_CATCH_HANDLE_H
#define LYRIC_ASSEMBLER_CATCH_HANDLE_H

#include "assembler_types.h"
#include "code_fragment.h"
#include "object_state.h"

namespace lyric_assembler {

    class CatchHandle {
    public:
        CatchHandle(
            const lyric_common::TypeDef &exceptionType,
            CodeFragment *fragment,
            ObjectState *state);

        lyric_common::TypeDef getExceptionType() const;

        CodeFragment *catchFragment() const;
        ObjectState *objectState() const;

    private:
        lyric_common::TypeDef m_exceptionType;
        CodeFragment *m_fragment;
        ObjectState *m_state;
    };
}

#endif // LYRIC_ASSEMBLER_CATCH_HANDLE_H