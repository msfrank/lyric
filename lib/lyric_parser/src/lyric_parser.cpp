#include <string>

#include <antlr4-runtime.h>

#include <ModuleLexer.h>
#include <ModuleParser.h>

#include <lyric_parser/archetype_state.h>
#include <lyric_parser/internal/module_archetype.h>
#include <lyric_parser/internal/tracing_error_listener.h>
#include <lyric_parser/lyric_archetype.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/parse_result.h>
#include <tempo_tracing/enter_scope.h>

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
    antlr4::ANTLRInputStream input(utf8.data(), utf8.size());
    ModuleLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    ModuleParser parser(&tokens);

    // create the trace context
    std::shared_ptr<tempo_tracing::TraceContext> context;
    if (recorder != nullptr) {
        TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeUnownedContextAndSwitch(recorder));
    } else {
        TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeContextAndSwitch());
    }

    // ensure context is released
    tempo_tracing::ReleaseContext releaser(context);

    // create the root span
    tempo_tracing::EnterScope scope("lyric_parser::LyricParser");

    // create the state
    ArchetypeState state(sourceUrl);

    // create the listener
    internal::ModuleArchetype listener(&state, context);

    // create the error listener
    internal::TracingErrorListener tracingErrorListener(&listener);

    lexer.removeErrorListeners();
    lexer.addErrorListener(&tracingErrorListener);
    parser.removeErrorListeners();
    parser.addErrorListener(&tracingErrorListener);

    try {
        antlr4::tree::ParseTree *tree = parser.root();
        antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    } catch (antlr4::ParseCancellationException &ex) {
        return ParseStatus::forCondition(ParseCondition::kParseInvariant, ex.what());
    } catch (std::exception &ex) {
        return ParseStatus::forCondition(ParseCondition::kParseInvariant, ex.what());
    }

    return listener.toArchetype();
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
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder)
{
    antlr4::ANTLRInputStream input(utf8.data(), utf8.size());
    ModuleLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    ModuleParser parser(&tokens);

    // create the trace context
    std::shared_ptr<tempo_tracing::TraceContext> context;
    if (recorder != nullptr) {
        TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeUnownedContextAndSwitch(recorder));
    } else {
        TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeContextAndSwitch());
    }

    // ensure context is released
    tempo_tracing::ReleaseContext releaser(context);

    // create the root span
    tempo_tracing::EnterScope scope("lyric_parser::LyricParser");

    // create the state
    ArchetypeState state(sourceUrl);

    // create the listener
    internal::ModuleArchetype listener(&state, context);

    // create the error listener
    internal::TracingErrorListener tracingErrorListener(&listener);

    lexer.removeErrorListeners();
    lexer.addErrorListener(&tracingErrorListener);
    parser.removeErrorListeners();
    parser.addErrorListener(&tracingErrorListener);

    try {
        antlr4::tree::ParseTree *tree = parser.block();
        antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    } catch (antlr4::ParseCancellationException &ex) {
        return ParseStatus::forCondition(ParseCondition::kParseInvariant, ex.what());
    }

    return listener.toArchetype();
}

/**
 * Parse the given source code for a defclass statement and generate its intermediate representation.
 *
 * @param utf8 A utf-8 encoded string containing the defclass source code.
 * @param sourceUrl The url of the source code location.
 * @param recorder A TraceRecorder.
 * @return An Archetype containing the defclass IR.
 */
tempo_utils::Result<lyric_parser::LyricArchetype>
lyric_parser::LyricParser::parseClass(
    std::string_view utf8,
    const tempo_utils::Url &sourceUrl,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder)
{
    antlr4::ANTLRInputStream input(utf8.data(), utf8.size());
    ModuleLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    ModuleParser parser(&tokens);

    // create the trace context
    std::shared_ptr<tempo_tracing::TraceContext> context;
    if (recorder != nullptr) {
        TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeUnownedContextAndSwitch(recorder));
    } else {
        TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeContextAndSwitch());
    }

    // ensure context is released
    tempo_tracing::ReleaseContext releaser(context);

    // create the root span
    tempo_tracing::EnterScope scope("lyric_parser::LyricParser");

    // create the state
    ArchetypeState state(sourceUrl);

    // create the listener
    internal::ModuleArchetype listener(&state, context);

    // create the error listener
    internal::TracingErrorListener tracingErrorListener(&listener);

    lexer.removeErrorListeners();
    lexer.addErrorListener(&tracingErrorListener);
    parser.removeErrorListeners();
    parser.addErrorListener(&tracingErrorListener);

    try {
        antlr4::tree::ParseTree *tree = parser.defclassStatement();
        antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    } catch (antlr4::ParseCancellationException &ex) {
        return ParseStatus::forCondition(ParseCondition::kParseInvariant, ex.what());
    }

    return listener.toArchetype();
}

/**
 * Parse the given source code for a defconcept statement and generate its intermediate representation.
 *
 * @param utf8 A utf-8 encoded string containing the defconcept source code.
 * @param sourceUrl The url of the source code location.
 * @param recorder A TraceRecorder.
 * @return An Archetype containing the defconcept IR.
 */
tempo_utils::Result<lyric_parser::LyricArchetype>
lyric_parser::LyricParser::parseConcept(
    std::string_view utf8,
    const tempo_utils::Url &sourceUrl,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder)
{
    antlr4::ANTLRInputStream input(utf8.data(), utf8.size());
    ModuleLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    ModuleParser parser(&tokens);

    // create the trace context
    std::shared_ptr<tempo_tracing::TraceContext> context;
    if (recorder != nullptr) {
        TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeUnownedContextAndSwitch(recorder));
    } else {
        TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeContextAndSwitch());
    }

    // ensure context is released
    tempo_tracing::ReleaseContext releaser(context);

    // create the root span
    tempo_tracing::EnterScope scope("lyric_parser::LyricParser");

    // create the state
    ArchetypeState state(sourceUrl);

    // create the listener
    internal::ModuleArchetype listener(&state, context);

    // create the error listener
    internal::TracingErrorListener tracingErrorListener(&listener);

    lexer.removeErrorListeners();
    lexer.addErrorListener(&tracingErrorListener);
    parser.removeErrorListeners();
    parser.addErrorListener(&tracingErrorListener);

    try {
        antlr4::tree::ParseTree *tree = parser.defconceptStatement();
        antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    } catch (antlr4::ParseCancellationException &ex) {
        return ParseStatus::forCondition(ParseCondition::kParseInvariant, ex.what());
    }

    return listener.toArchetype();
}

/**
 * Parse the given source code for a defenum statement and generate its intermediate representation.
 *
 * @param utf8 A utf-8 encoded string containing the defenum source code.
 * @param sourceUrl The url of the source code location.
 * @param recorder A TraceRecorder.
 * @return An Archetype containing the defenum IR.
 */
tempo_utils::Result<lyric_parser::LyricArchetype>
lyric_parser::LyricParser::parseEnum(
    std::string_view utf8,
    const tempo_utils::Url &sourceUrl,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder)
{
    antlr4::ANTLRInputStream input(utf8.data(), utf8.size());
    ModuleLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    ModuleParser parser(&tokens);

    // create the trace context
    std::shared_ptr<tempo_tracing::TraceContext> context;
    if (recorder != nullptr) {
        TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeUnownedContextAndSwitch(recorder));
    } else {
        TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeContextAndSwitch());
    }

    // ensure context is released
    tempo_tracing::ReleaseContext releaser(context);

    // create the root span
    tempo_tracing::EnterScope scope("lyric_parser::LyricParser");

    // create the state
    ArchetypeState state(sourceUrl);

    // create the listener
    internal::ModuleArchetype listener(&state, context);

    // create the error listener
    internal::TracingErrorListener tracingErrorListener(&listener);

    lexer.removeErrorListeners();
    lexer.addErrorListener(&tracingErrorListener);
    parser.removeErrorListeners();
    parser.addErrorListener(&tracingErrorListener);

    try {
        antlr4::tree::ParseTree *tree = parser.defenumStatement();
        antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    } catch (antlr4::ParseCancellationException &ex) {
        return ParseStatus::forCondition(ParseCondition::kParseInvariant, ex.what());
    }

    return listener.toArchetype();
}

/**
 * Parse the given source code for a def statement and generate its intermediate representation.
 *
 * @param utf8 A utf-8 encoded string containing the def source code.
 * @param sourceUrl The url of the source code location.
 * @param recorder A TraceRecorder.
 * @return An Archetype containing the def IR.
 */
tempo_utils::Result<lyric_parser::LyricArchetype>
lyric_parser::LyricParser::parseFunction(
    std::string_view utf8,
    const tempo_utils::Url &sourceUrl,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder)
{
    antlr4::ANTLRInputStream input(utf8.data(), utf8.size());
    ModuleLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    ModuleParser parser(&tokens);

    // create the trace context
    std::shared_ptr<tempo_tracing::TraceContext> context;
    if (recorder != nullptr) {
        TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeUnownedContextAndSwitch(recorder));
    } else {
        TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeContextAndSwitch());
    }

    // ensure context is released
    tempo_tracing::ReleaseContext releaser(context);

    // create the root span
    tempo_tracing::EnterScope scope("lyric_parser::LyricParser");

    // create the state
    ArchetypeState state(sourceUrl);

    // create the listener
    internal::ModuleArchetype listener(&state, context);

    // create the error listener
    internal::TracingErrorListener tracingErrorListener(&listener);

    lexer.removeErrorListeners();
    lexer.addErrorListener(&tracingErrorListener);
    parser.removeErrorListeners();
    parser.addErrorListener(&tracingErrorListener);

    try {
        antlr4::tree::ParseTree *tree = parser.defStatement();
        antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    } catch (antlr4::ParseCancellationException &ex) {
        return ParseStatus::forCondition(ParseCondition::kParseInvariant, ex.what());
    }

    return listener.toArchetype();
}

/**
 * Parse the given source code for a definstance statement and generate its intermediate representation.
 *
 * @param utf8 A utf-8 encoded string containing the definstance source code.
 * @param sourceUrl The url of the source code location.
 * @param recorder A TraceRecorder.
 * @return An Archetype containing the definstance IR.
 */
tempo_utils::Result<lyric_parser::LyricArchetype>
lyric_parser::LyricParser::parseInstance(
    std::string_view utf8,
    const tempo_utils::Url &sourceUrl,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder)
{
    antlr4::ANTLRInputStream input(utf8.data(), utf8.size());
    ModuleLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    ModuleParser parser(&tokens);

    // create the trace context
    std::shared_ptr<tempo_tracing::TraceContext> context;
    if (recorder != nullptr) {
        TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeUnownedContextAndSwitch(recorder));
    } else {
        TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeContextAndSwitch());
    }

    // ensure context is released
    tempo_tracing::ReleaseContext releaser(context);

    // create the root span
    tempo_tracing::EnterScope scope("lyric_parser::LyricParser");

    // create the state
    ArchetypeState state(sourceUrl);

    // create the listener
    internal::ModuleArchetype listener(&state, context);

    // create the error listener
    internal::TracingErrorListener tracingErrorListener(&listener);

    lexer.removeErrorListeners();
    lexer.addErrorListener(&tracingErrorListener);
    parser.removeErrorListeners();
    parser.addErrorListener(&tracingErrorListener);

    try {
        antlr4::tree::ParseTree *tree = parser.definstanceStatement();
        antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    } catch (antlr4::ParseCancellationException &ex) {
        return ParseStatus::forCondition(ParseCondition::kParseInvariant, ex.what());
    }

    return listener.toArchetype();
}

/**
 * Parse the given source code for a defstruct statement and generate its intermediate representation.
 *
 * @param utf8 A utf-8 encoded string containing the defstruct source code.
 * @param sourceUrl The url of the source code location.
 * @param recorder A TraceRecorder.
 * @return An Archetype containing the defstruct IR.
 */
tempo_utils::Result<lyric_parser::LyricArchetype>
lyric_parser::LyricParser::parseStruct(
    std::string_view utf8,
    const tempo_utils::Url &sourceUrl,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder)
{
    antlr4::ANTLRInputStream input(utf8.data(), utf8.size());
    ModuleLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    ModuleParser parser(&tokens);

    // create the trace context
    std::shared_ptr<tempo_tracing::TraceContext> context;
    if (recorder != nullptr) {
        TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeUnownedContextAndSwitch(recorder));
    } else {
        TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeContextAndSwitch());
    }

    // ensure context is released
    tempo_tracing::ReleaseContext releaser(context);

    // create the root span
    tempo_tracing::EnterScope scope("lyric_parser::LyricParser");

    // create the state
    ArchetypeState state(sourceUrl);

    // create the listener
    internal::ModuleArchetype listener(&state, context);

    // create the error listener
    internal::TracingErrorListener tracingErrorListener(&listener);

    lexer.removeErrorListeners();
    lexer.addErrorListener(&tracingErrorListener);
    parser.removeErrorListeners();
    parser.addErrorListener(&tracingErrorListener);

    try {
        antlr4::tree::ParseTree *tree = parser.defstructStatement();
        antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    } catch (antlr4::ParseCancellationException &ex) {
        return ParseStatus::forCondition(ParseCondition::kParseInvariant, ex.what());
    }

    return listener.toArchetype();
}
