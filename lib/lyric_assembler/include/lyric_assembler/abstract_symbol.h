#ifndef LYRIC_ASSEMBLER_ABSTRACT_SYMBOL_H
#define LYRIC_ASSEMBLER_ABSTRACT_SYMBOL_H

#include <lyric_common/symbol_url.h>

#include "assembler_types.h"

namespace lyric_assembler {

    enum class SymbolType {
        INTRINSIC,
        EXISTENTIAL,
        ACTION,
        CALL,
        CONSTANT,
        FIELD,
        STATIC,
        LOCAL,
        LEXICAL,
        ARGUMENT,
        SYNTHETIC,
        CONCEPT,
        CLASS,
        INSTANCE,
        STRUCT,
        ENUM,
        NAMESPACE,
        UNDECLARED,
    };

    class AbstractSymbol {
    public:
        virtual ~AbstractSymbol() = default;
        virtual bool isImported() const = 0;
        virtual SymbolType getSymbolType() const = 0;
        virtual lyric_common::SymbolUrl getSymbolUrl() const = 0;
        virtual lyric_common::TypeDef getAssignableType() const = 0;
    };
}

#endif // LYRIC_ASSEMBLER_ABSTRACT_SYMBOL_H
