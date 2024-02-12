#ifndef LYRIC_PARSER_INTERNAL_LEXER_ERROR_LISTENER_H
#define LYRIC_PARSER_INTERNAL_LEXER_ERROR_LISTENER_H

#include <memory>

#include <antlr4-runtime.h>

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class LexerErrorListener : public antlr4::BaseErrorListener {

    public:
        explicit LexerErrorListener(ArchetypeState *state);

        void syntaxError(
            antlr4::Recognizer *recognizer,
            antlr4::Token *offendingSymbol,
            size_t line,
            size_t charPositionInLine,
            const std::string &message,
            std::exception_ptr e) override;

    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_LEXER_ERROR_LISTENER_H
