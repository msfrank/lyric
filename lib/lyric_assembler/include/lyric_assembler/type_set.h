#ifndef LYRIC_ASSEMBLER_TYPE_SET_H
#define LYRIC_ASSEMBLER_TYPE_SET_H

#include "assembly_state.h"

namespace lyric_assembler {

    struct TypeLevel {
        TypeAddress address;
        lyric_common::TypeDef typeDef;
        std::vector<TypeLevel *> children;
        ~TypeLevel();
    };

    class DisjointTypeSet {

    public:
        explicit DisjointTypeSet(const lyric_assembler::AssemblyState *state);
        ~DisjointTypeSet();

        tempo_utils::Status putType(const lyric_common::TypeDef &type);

    private:
        const AssemblyState *m_state;
        TypeLevel *m_root;
    };

    class UnifiedTypeSet {

    public:
        explicit UnifiedTypeSet(const lyric_assembler::AssemblyState *state);
        ~UnifiedTypeSet();

        tempo_utils::Status putType(const lyric_common::TypeDef &type);
        lyric_common::TypeDef getUnifiedType() const;

    private:
        const AssemblyState *m_state;
        TypeLevel *m_root;
    };
}

#endif // LYRIC_ASSEMBLER_TYPE_SET_H