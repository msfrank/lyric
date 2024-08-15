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
            TypeAddress address,
            TypeHandle *superType,
            ObjectState *state);
        TypeHandle(
            int placeholderIndex,
            const lyric_common::SymbolUrl &templateUrl,
            const std::vector<lyric_common::TypeDef> &typeArguments,
            TypeAddress address,
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
        TypeAddress getAddress() const;
        TypeSignature getTypeSignature() const;

        void touch();

        tempo_utils::Status updateAddress(TypeAddress address);

    private:
        lyric_common::TypeDef m_typeDef;
        TypeHandle *m_superType;
        TypeAddress m_address;
        TypeSignature m_signature;
        ObjectState *m_state;
    };
}

#endif // LYRIC_ASSEMBLER_TYPE_HANDLE_H
