#include <string>

#include <antlr4-runtime.h>

#include <ModuleLexer.h>
#include <ModuleParser.h>

#include <lyric_parser/archetype_state.h>
#include <lyric_parser/internal/lexer_error_listener.h>
#include <lyric_parser/internal/module_archetype.h>
#include <lyric_parser/internal/parser_error_listener.h>
#include <lyric_parser/lyric_archetype.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/parse_result.h>

lyric_parser::LyricParser::LyricParser(const ParserOptions &options)
    : m_options(options)
{
}

lyric_parser::LyricParser::LyricParser(const LyricParser &other)
    : m_options(other.m_options)
{
}

/**
 * Parse the given source code for a module an generate its intermediate representation.
 *
 * @param utf8 A utf-8 encoded string containing the module source code.
 * @param sourceUrl The url of the source code location.
 * @param recorder A TraceRecorder.
 * @return An Archetype containing the module IR.
 */
tempo_utils::Result<lyric_parser::LyricArchetype>
lyric_parser::LyricParser::parseModule(
    std::string_view utf8,
    const tempo_utils::Url &sourceUrl,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder)
{
    TU_ASSERT (recorder != nullptr);

    antlr4::ANTLRInputStream input(utf8.data(), utf8.size());
    ModuleLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    ModuleParser parser(&tokens);

    // create a new span
    tempo_tracing::ScopeManager scopeManager(recorder);
    auto span = scopeManager.makeSpan();
    span->setOperationName("parseModule");

    ArchetypeState state(sourceUrl, &scopeManager);
    internal::ModuleArchetype listener(&state);

    internal::LexerErrorListener lexerErrorListener(&state);
    lexer.removeErrorListeners();
    lexer.addErrorListener(&lexerErrorListener);

    internal::ParserErrorListener parserErrorListener(&state);
    parser.removeErrorListeners();
    parser.addErrorListener(&parserErrorListener);
    auto handler = std::make_shared<internal::ParserErrorStrategy>(&state);
    parser.setErrorHandler(handler);

    try {
        antlr4::tree::ParseTree *tree = parser.root();
        antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    } catch (ParseException &ex) {
        return ex.getStatus();
    } catch (antlr4::ParseCancellationException &ex) {
        return ParseStatus::forCondition(ParseCondition::kParseInvariant, ex.what());
    }

    auto toArchetypeResult = state.toArchetype();
    if (toArchetypeResult.isStatus())
        return toArchetypeResult.getStatus();
    return toArchetypeResult.getResult();
}

/**
 * Parse the given source code for a code block and generate its intermediate representation.
 *
 * @param utf8 A utf-8 encoded string containing the block source code.
 * @param sourceUrl The url of the source code location.
 * @param recorder A TraceRecorder.
 * @return An Archetype containing the block IR.
 */
tempo_utils::Result<lyric_parser::LyricArchetype>
lyric_parser::LyricParser::parseBlock(
    std::string_view utf8,
    const tempo_utils::Url &sourceUrl,
    tempo_tracing::ScopeManager *scopeManager)
{
    TU_ASSERT (scopeManager != nullptr);

    antlr4::ANTLRInputStream input(utf8.data(), utf8.size());
    ModuleLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    ModuleParser parser(&tokens);

    // create a new span
    auto span = scopeManager->makeSpan();
    span->setOperationName("parseBlock");

    ArchetypeState state(sourceUrl, scopeManager);
    internal::ModuleArchetype listener(&state);

    internal::LexerErrorListener lexerErrorListener(&state);
    lexer.removeErrorListeners();
    lexer.addErrorListener(&lexerErrorListener);

    internal::ParserErrorListener parserErrorListener(&state);
    parser.removeErrorListeners();
    parser.addErrorListener(&parserErrorListener);
    auto handler = std::make_shared<internal::ParserErrorStrategy>(&state);
    parser.setErrorHandler(handler);

    try {
        antlr4::tree::ParseTree *tree = parser.block();
        antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    } catch (ParseException &ex) {
        return ex.getStatus();
    } catch (antlr4::ParseCancellationException &ex) {
        return ParseStatus::forCondition(ParseCondition::kParseInvariant, ex.what());
    }

    auto toArchetypeResult = state.toArchetype();
    if (toArchetypeResult.isStatus())
        return toArchetypeResult.getStatus();
    return toArchetypeResult.getResult();
}
