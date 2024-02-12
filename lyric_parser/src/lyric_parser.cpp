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

    tempo_tracing::ScopeManager scopeManager(recorder);
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

tempo_utils::Result<lyric_parser::LyricArchetype>
lyric_parser::LyricParser::parseStruct(
    std::string_view utf8,
    const tempo_utils::Url &sourceUrl,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder)
{
    TU_ASSERT (recorder != nullptr);
    return tempo_utils::GenericStatus::forCondition(
        tempo_utils::GenericCondition::kUnimplemented, "parseStruct");

//    antlr4::ANTLRInputStream input(utf8.data(), utf8.size());
//    StructLexer lexer(&input);
//    antlr4::CommonTokenStream tokens(&lexer);
//    StructParser parser(&tokens);
//
//    ArchetypeState state(sourcePath, scopeManager);
//    StructArchetype listener(&state);
//
//    LexerErrorListener lexerErrorListener(&state);
//    lexer.removeErrorListeners();
//    lexer.addErrorListener(&lexerErrorListener);
//
//    ParserErrorListener parserErrorListener(&state);
//    parser.removeErrorListeners();
//    parser.addErrorListener(&parserErrorListener);
//    auto handler = std::make_shared<ParserErrorStrategy>();
//    parser.setErrorHandler(handler);
//
//    try {
//        antlr4::tree::ParseTree *tree = parser.root();
//        antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
//    } catch (IncompleteModuleException &ex) {
//        return ParseResult<LyricArchetype>(ParseStatus::incompleteModule());
//    } catch (SyntaxErrorException &ex) {
//        return ParseResult<LyricArchetype>(
//            ParseStatus::syntaxError(ex.getMessage()));
//    } catch (ParseInvariantException &ex) {
//        return ParseResult<LyricArchetype>(
//            ParseStatus::internalViolation(ex.getMessage()));
//    } catch (InternalViolationException &ex) {
//        return ParseResult<LyricArchetype>(
//            ParseStatus::internalViolation(ex.getMessage()));
//    } catch (antlr4::ParseCancellationException &ex) {
//        return ParseResult<LyricArchetype>(
//            ParseStatus::internalViolation(ex.what()));
//    }
//
//    return state.toArchetype();
}