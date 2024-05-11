
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/internal/parser_error_listener.h>
#include <tempo_utils/log_message.h>

lyric_parser::internal::ParserErrorListener::ParserErrorListener(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ParserErrorListener::syntaxError(
    antlr4::Recognizer *recognizer,
    antlr4::Token *offendingSymbol,
    size_t line,
    size_t charPositionInLine,
    const std::string &message,
    std::exception_ptr e)
{
    // FIXME: could this ever be nullptr?
    if (!offendingSymbol)
        throw tempo_utils::StatusException(
            tempo_utils::Status(tempo_utils::StatusCode::kInternal, message));

    // if match failed due to end-of-file, then throw IncompleteModuleException
    if (offendingSymbol->getType() == antlr4::Token::EOF)
        m_state->throwIncompleteModule(offendingSymbol);

    // FIXME: when does antlr4 raise a syntax error with an empty exception?
    if (!e) {
        throw tempo_utils::StatusException(
            tempo_utils::Status(tempo_utils::StatusCode::kInternal, message));
    }

    // convert antlr4 exceptions into our own exceptions
    try {
        std::rethrow_exception(e);
    } catch(antlr4::FailedPredicateException &ex) {
        m_state->throwSyntaxError(offendingSymbol, message);
    } catch(antlr4::InputMismatchException &ex) {
        m_state->throwSyntaxError(offendingSymbol, message);
    } catch(antlr4::NoViableAltException &ex) {
        m_state->throwSyntaxError(offendingSymbol, message);
    } catch(antlr4::RuntimeException &ex) {
        throw tempo_utils::StatusException(
            tempo_utils::Status(tempo_utils::StatusCode::kInternal, message));
    }
}

lyric_parser::internal::ParserErrorStrategy::ParserErrorStrategy(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ParserErrorStrategy::recover(antlr4::Parser *recognizer, std::exception_ptr e)
{
    m_state->throwSyntaxError(recognizer->getCurrentToken(),"parse failure on recover");
}

antlr4::Token *
lyric_parser::internal::ParserErrorStrategy::recoverInline(antlr4::Parser *recognizer)
{
    m_state->throwSyntaxError(recognizer->getCurrentToken(),"parse failure on recoverInline");
    TU_UNREACHABLE();
}

void
lyric_parser::internal::ParserErrorStrategy::sync(antlr4::Parser *recognizer)
{
}
