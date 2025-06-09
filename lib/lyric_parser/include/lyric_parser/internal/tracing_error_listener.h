#ifndef LYRIC_PARSER_INTERNAL_TRACING_ERROR_LISTENER_H
#define LYRIC_PARSER_INTERNAL_TRACING_ERROR_LISTENER_H

#include <antlr4-runtime.h>

#include "module_archetype.h"

namespace lyric_parser::internal {

    class TracingErrorListener : public antlr4::BaseErrorListener {

    public:
        explicit TracingErrorListener(ModuleArchetype *archetype);
        void syntaxError(
            antlr4::Recognizer *recognizer,
            antlr4::Token *offendingSymbol,
            size_t line,
            size_t charPositionInLine,
            const std::string &message,
            std::exception_ptr e) override;

    private:
        ModuleArchetype *m_archetype;
    };
}

#endif // LYRIC_PARSER_INTERNAL_TRACING_ERROR_LISTENER_H
