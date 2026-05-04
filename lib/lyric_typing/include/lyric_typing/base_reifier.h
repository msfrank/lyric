#ifndef LYRIC_TYPING_BASE_REIFIER_H
#define LYRIC_TYPING_BASE_REIFIER_H
#include "lyric_assembler/object_state.h"

namespace lyric_typing {

    class BaseReifier {
    public:
        explicit BaseReifier(lyric_assembler::ObjectState *state);

    protected:
        tempo_utils::Status checkPlaceholder(
            const lyric_object::TemplateParameter &tp,
            const lyric_common::TypeDef &arg) const;

        tempo_utils::Result<lyric_common::TypeDef> reifySingular(
            const lyric_common::TypeDef &paramType);

        tempo_utils::Result<lyric_common::TypeDef> reifyUnion(
            const lyric_common::TypeDef &paramType);

    private:
        lyric_assembler::ObjectState *m_state;
    };
}
#endif // LYRIC_TYPING_BASE_REIFIER_H
