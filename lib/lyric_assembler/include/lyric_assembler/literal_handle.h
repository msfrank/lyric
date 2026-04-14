#ifndef LYRIC_ASSEMBLER_LITERAL_HANDLE_H
#define LYRIC_ASSEMBLER_LITERAL_HANDLE_H

#include "assembler_types.h"

namespace lyric_assembler {

    class LiteralHandle {

    public:
        explicit LiteralHandle(std::string literal);

        std::string getLiteral() const;
        std::string_view literalValue() const;

    private:
        std::string m_literal;
    };
}

#endif // LYRIC_ASSEMBLER_LITERAL_HANDLE_H
