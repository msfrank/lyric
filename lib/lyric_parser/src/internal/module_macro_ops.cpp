
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
lyric_parser::internal::ModuleMacroOps::exitMacroArgs(ModuleParser::MacroArgsContext *ctx)
{
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));

    if (ctx->argList()) {
        auto *argList = ctx->argList();

        std::deque<ArchetypeNode *> argNodes;
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

            argNodes.push_front(argNode);
        }

        auto *macroNode = m_state->peekNode();
        for (auto *argNode : argNodes) {
            macroNode->appendChild(argNode);
        }
    }
}

void
lyric_parser::internal::ModuleMacroOps::enterMacroCall(ModuleParser::MacroCallContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *macroCallNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstMacroCallClass, location);

    auto id = ctx->Identifier()->getText();
    macroCallNode->putAttr(kLyricAstIdentifier, id);

    m_state->pushNode(macroCallNode);
}

void
lyric_parser::internal::ModuleMacroOps::exitMacroCall(ModuleParser::MacroCallContext *ctx)
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
lyric_parser::internal::ModuleMacroOps::enterMacroAnnotation(ModuleParser::MacroAnnotationContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *macroCallNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstMacroCallClass, location);

    auto id = ctx->Identifier()->getText();
    macroCallNode->putAttr(kLyricAstIdentifier, id);

    m_state->pushNode(macroCallNode);
}

void
lyric_parser::internal::ModuleMacroOps::exitMacroAnnotation(ModuleParser::MacroAnnotationContext *ctx)
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
lyric_parser::internal::ModuleMacroOps::enterPragmaMacro(ModuleParser::PragmaMacroContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *pragmaNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstPragmaClass, location);

    auto id = ctx->Identifier()->getText();
    pragmaNode->putAttr(kLyricAstIdentifier, id);

    m_state->pushNode(pragmaNode);
}

void
lyric_parser::internal::ModuleMacroOps::exitPragmaMacro(ModuleParser::PragmaMacroContext *ctx)
{
    auto *pragmaNode = m_state->popNode();
    m_state->checkNodeOrThrow(pragmaNode, lyric_schema::kLyricAstPragmaClass);
    m_state->addPragma(pragmaNode);
}

void
lyric_parser::internal::ModuleMacroOps::enterDefinitionMacro(ModuleParser::DefinitionMacroContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *macroListNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstMacroListClass, location);
    m_state->pushNode(macroListNode);
}

void
lyric_parser::internal::ModuleMacroOps::exitDefinitionMacro(ModuleParser::DefinitionMacroContext *ctx)
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
lyric_parser::internal::ModuleMacroOps::enterBlockMacro(ModuleParser::BlockMacroContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *macroListNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstMacroListClass, location);
    m_state->pushNode(macroListNode);
}

void
lyric_parser::internal::ModuleMacroOps::exitBlockMacro(ModuleParser::BlockMacroContext *ctx)
{
    // the macro list
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *macroListNode = m_state->peekNode();
    m_state->checkNodeOrThrow(macroListNode, lyric_schema::kLyricAstMacroListClass);
}
