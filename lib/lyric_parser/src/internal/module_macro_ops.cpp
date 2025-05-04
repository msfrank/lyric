
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_macro_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleMacroOps::ModuleMacroOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleMacroOps::exitRewriteArgs(ModuleParser::RewriteArgsContext *ctx)
{
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *ruleNode = m_state->peekNode();

    if (ctx->argList()) {
        auto *argList = ctx->argList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argSpec(i);
            if (argSpec == nullptr)
                continue;

            if (m_state->isEmpty())
                m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
            auto *argNode = m_state->popNode();

            if (argSpec->Identifier() != nullptr) {
                auto label = argSpec->Identifier()->getText();

                auto token = argSpec->getStart();
                auto location = get_token_location(token);

                auto *keywordNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstKeywordClass, location);
                keywordNode->putAttr(kLyricAstIdentifier, label);
                keywordNode->appendChild(argNode);
                argNode = keywordNode;
            }

            ruleNode->prependChild(argNode);
        }
    }
}

void
lyric_parser::internal::ModuleMacroOps::enterPragma(ModuleParser::PragmaContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *pragmaNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstPragmaClass, location);

    auto id = ctx->Identifier()->getText();
    pragmaNode->putAttr(kLyricAstIdentifier, id);

    m_state->pushNode(pragmaNode);
}

void
lyric_parser::internal::ModuleMacroOps::exitPragma(ModuleParser::PragmaContext *ctx)
{
    auto *pragmaNode = m_state->popNode();
    m_state->checkNodeOrThrow(pragmaNode, lyric_schema::kLyricAstPragmaClass);
    m_state->addPragma(pragmaNode);
}

void
lyric_parser::internal::ModuleMacroOps::enterAnnotation(ModuleParser::AnnotationContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *macroCallNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstMacroCallClass, location);

    auto id = ctx->Identifier()->getText();
    macroCallNode->putAttr(kLyricAstIdentifier, id);

    m_state->pushNode(macroCallNode);
}

void
lyric_parser::internal::ModuleMacroOps::exitAnnotation(ModuleParser::AnnotationContext *ctx)
{
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *macroCallNode = m_state->popNode();
    m_state->checkNodeOrThrow(macroCallNode, lyric_schema::kLyricAstMacroCallClass);

    // if ancestor node is not a kMacroList, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *macroListNode = m_state->peekNode();
    m_state->checkNodeOrThrow(macroListNode, lyric_schema::kLyricAstMacroListClass);

    // otherwise append call to the macro list
    macroListNode->appendChild(macroCallNode);
}

void
lyric_parser::internal::ModuleMacroOps::enterMacro(ModuleParser::MacroContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *macroCallNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstMacroCallClass, location);

    auto id = ctx->Identifier()->getText();
    macroCallNode->putAttr(kLyricAstIdentifier, id);

    m_state->pushNode(macroCallNode);
}

void
lyric_parser::internal::ModuleMacroOps::exitMacro(ModuleParser::MacroContext *ctx)
{
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *macroCallNode = m_state->popNode();
    m_state->checkNodeOrThrow(macroCallNode, lyric_schema::kLyricAstMacroCallClass);

    // if ancestor node is not a kMacroList, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *macroListNode = m_state->peekNode();
    m_state->checkNodeOrThrow(macroListNode, lyric_schema::kLyricAstMacroListClass);

    // otherwise append call to the macro list
    macroListNode->appendChild(macroCallNode);
}

void
lyric_parser::internal::ModuleMacroOps::enterAnnotationList(ModuleParser::AnnotationListContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *macroListNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstMacroListClass, location);
    m_state->pushNode(macroListNode);
}

void
lyric_parser::internal::ModuleMacroOps::exitAnnotationList(ModuleParser::AnnotationListContext *ctx)
{
    // the macro list
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *macroListNode = m_state->popNode();
    m_state->checkNodeOrThrow(macroListNode, lyric_schema::kLyricAstMacroListClass);

    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *definitionNode = m_state->peekNode();
    TU_RAISE_IF_NOT_OK (definitionNode->putAttr(lyric_parser::kLyricAstMacroListOffset, macroListNode));
}

void
lyric_parser::internal::ModuleMacroOps::enterMacroList(ModuleParser::MacroListContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *macroListNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstMacroListClass, location);
    m_state->pushNode(macroListNode);
}

void
lyric_parser::internal::ModuleMacroOps::exitMacroList(ModuleParser::MacroListContext *ctx)
{
    // the macro list
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *macroListNode = m_state->peekNode();
    m_state->checkNodeOrThrow(macroListNode, lyric_schema::kLyricAstMacroListClass);
}
