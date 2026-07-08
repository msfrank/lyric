
#include <absl/strings/substitute.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/internal/tracing_error_listener.h>
#include <lyric_parser/internal/parser_utils.h>
#include <tempo_utils/log_message.h>

lyric_parser::internal::TracingErrorListener::TracingErrorListener(ModuleArchetype *archetype)
    : m_archetype(archetype)
{
    TU_ASSERT (m_archetype != nullptr);
}

void
lyric_parser::internal::TracingErrorListener::syntaxError(
    antlr4::Recognizer *recognizer,
    antlr4::Token *offendingSymbol,
    size_t line,
    size_t charPositionInLine,
    const std::string &message,
    std::exception_ptr e)
{
    if (offendingSymbol) {
        if (offendingSymbol->getType() == antlr4::Token::EOF) {
            m_archetype->logError(line, charPositionInLine, message);
            return;
        }
    }

    // we know exception is empty due to the following conditions:
    //  - antlr reporting an unwanted token which was resolved using single token deletion
    //  - antlr reporting a missing token which was resolved using single token insertion
    //  - antlr reporting ambiguity warnings
    //  - user error notifications via notifyErrorListeners()
    if (!e) {
        m_archetype->logError(line, charPositionInLine, message);
        return;
    }

    // log known antlr4 exceptions
    try {
        std::rethrow_exception(e);
    } catch(antlr4::InputMismatchException &ex) {
    } catch(antlr4::FailedPredicateException &ex) {
    } catch(antlr4::NoViableAltException &ex) {
    } catch(antlr4::LexerNoViableAltException &ex) {
    } catch(antlr4::RuntimeException &ex) {
        // if we encounter any other antlr exception then we exit parsing immediately
        throw tempo_utils::StatusException(
            ParseStatus::forCondition(ParseCondition::kParseInvariant, message));
    }

    // log user visible syntax error message
    m_archetype->logError(line, charPositionInLine, "syntax error");

    // log full exception message at VV severity
    auto fullMessage = absl::Substitute("$0:$1: $2", line, charPositionInLine, message);
    TU_LOG_VV << fullMessage;
}