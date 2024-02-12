#ifndef LYRIC_PARSER_INTERNAL_PARSER_ERROR_LISTENER_H
#define LYRIC_PARSER_INTERNAL_PARSER_ERROR_LISTENER_H

#include <memory>

#include <antlr4-runtime.h>

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ParserErrorListener : public antlr4::BaseErrorListener {

    public:
        explicit ParserErrorListener(ArchetypeState *state);
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

    class ParserErrorStrategy : public antlr4::DefaultErrorStrategy {

    public:
        explicit ParserErrorStrategy(ArchetypeState *state);
        void recover(antlr4::Parser *recognizer, std::exception_ptr e) override;
        antlr4::Token *recoverInline(antlr4::Parser *recognizer) override;
        void sync(antlr4::Parser *recognizer) override;

    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_PARSER_ERROR_LISTENER_H
