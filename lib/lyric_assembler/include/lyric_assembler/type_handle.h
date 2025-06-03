#ifndef LYRIC_ASSEMBLER_TYPE_HANDLE_H
#define LYRIC_ASSEMBLER_TYPE_HANDLE_H

#include <lyric_common/symbol_url.h>
#include <lyric_common/type_def.h>

#include "object_state.h"

namespace lyric_assembler {

    class TypeHandle {

    public:
        explicit TypeHandle(const lyric_common::TypeDef &typeDef);
        TypeHandle(const lyric_common::TypeDef &typeDef, TypeHandle *superType);
        TypeHandle(
            const lyric_common::SymbolUrl &concreteUrl,
            const std::vector<lyric_common::TypeDef> &typeArguments,
            TypeHandle *superType);
        TypeHandle(
            int placeholderIndex,
            const lyric_common::SymbolUrl &templateUrl,
            const std::vector<lyric_common::TypeDef> &typeArguments);
        TypeHandle(
            const lyric_common::SymbolUrl &specificUrl,
            const std::vector<lyric_common::TypeDef> &typeArguments,
            AbstractSymbol *specificSymbol);

        lyric_common::TypeDef getTypeDef() const;
        lyric_common::SymbolUrl getTypeSymbol() const;
        std::vector<lyric_common::TypeDef>::const_iterator typeArgumentsBegin() const;
        std::vector<lyric_common::TypeDef>::const_iterator typeArgumentsEnd() const;
        TypeHandle *getSuperType() const;

        tempo_utils::Status defineType(
            const std::vector<lyric_common::TypeDef> &typeArguments,
            TypeHandle *superType);

    private:
        lyric_common::TypeDef m_typeDef;
        TypeHandle *m_superType;
        AbstractSymbol *m_specificSymbol;
    };
}

#endif // LYRIC_ASSEMBLER_TYPE_HANDLE_H
