#ifndef LYRIC_ASSEMBLER_TYPE_HANDLE_H
#define LYRIC_ASSEMBLER_TYPE_HANDLE_H

#include <lyric_common/symbol_url.h>
#include <lyric_common/type_def.h>
#include <lyric_object/object_types.h>

#include "object_state.h"

namespace lyric_assembler {

    class TypeHandle {

    public:
        TypeHandle(
            const lyric_common::SymbolUrl &concreteUrl,
            const std::vector<lyric_common::TypeDef> &typeArguments,
            TypeHandle *superType,
            ObjectState *state);
        TypeHandle(
            int placeholderIndex,
            const lyric_common::SymbolUrl &templateUrl,
            const std::vector<lyric_common::TypeDef> &typeArguments,
            ObjectState *state);
        TypeHandle(
            const lyric_common::TypeDef &typeDef,
            TypeHandle *superType,
            ObjectState *state);

        lyric_common::TypeDef getTypeDef() const;
        lyric_common::SymbolUrl getTypeSymbol() const;
        std::vector<lyric_common::TypeDef>::const_iterator typeArgumentsBegin() const;
        std::vector<lyric_common::TypeDef>::const_iterator typeArgumentsEnd() const;
        TypeHandle *getSuperType() const;

    private:
        lyric_common::TypeDef m_typeDef;
        TypeHandle *m_superType;
        TypeSignature m_signature;
    };
}

#endif // LYRIC_ASSEMBLER_TYPE_HANDLE_H
