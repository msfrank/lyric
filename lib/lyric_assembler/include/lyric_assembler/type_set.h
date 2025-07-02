#ifndef LYRIC_ASSEMBLER_TYPE_SET_H
#define LYRIC_ASSEMBLER_TYPE_SET_H

#include "object_state.h"

namespace lyric_assembler {

    struct TypeLevel {
        const TypeHandle *typeHandle;
        lyric_common::TypeDef typeDef;
        std::vector<TypeLevel *> children;
        ~TypeLevel();
    };

    class DisjointTypeSet {

    public:
        explicit DisjointTypeSet(const ObjectState *state);
        ~DisjointTypeSet();

        tempo_utils::Status putType(const lyric_common::TypeDef &type);

    private:
        const ObjectState *m_state;
        TypeLevel *m_root;
    };

    class UnifiedTypeSet {

    public:
        explicit UnifiedTypeSet(const ObjectState *state);
        ~UnifiedTypeSet();

        tempo_utils::Status putType(const lyric_common::TypeDef &type);
        lyric_common::TypeDef getUnifiedType() const;

    private:
        const ObjectState *m_state;
        TypeLevel *m_root;
    };
}

#endif // LYRIC_ASSEMBLER_TYPE_SET_H