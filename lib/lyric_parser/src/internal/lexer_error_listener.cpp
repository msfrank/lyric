
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/internal/lexer_error_listener.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parse_result.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::LexerErrorListener::LexerErrorListener(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::LexerErrorListener::syntaxError(
    antlr4::Recognizer *recognizer,
    antlr4::Token *offendingSymbol,
    size_t line,
    size_t charPositionInLine,
    const std::string &message,
    std::exception_ptr e)
{
    if (!e) {
        throw tempo_utils::StatusException(
            tempo_utils::Status(tempo_utils::StatusCode::kInternal, message));
    }

    try {
        std::rethrow_exception(e);
    } catch(antlr4::LexerNoViableAltException &ex) {
        auto location = get_token_location(offendingSymbol);
        m_state->throwSyntaxError(location, message);
    } catch(antlr4::RuntimeException &ex) {
        throw tempo_utils::StatusException(
            tempo_utils::Status(tempo_utils::StatusCode::kInternal, message));
    }
}