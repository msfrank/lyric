
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_macro_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleMacroOps::ModuleMacroOps(ModuleArchetype *listener)
    : BaseOps(listener)
{
}

void
lyric_parser::internal::ModuleMacroOps::exitMacroArgs(ModuleParser::MacroArgsContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    if (ctx->argumentList()) {
        auto *argList = ctx->argumentList();

        std::deque<ArchetypeNode *> argNodes;
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argument(i);
            if (argSpec == nullptr)
                continue;

            ArchetypeNode *argNode;
            TU_ASSIGN_OR_RAISE (argNode, state->popNode());

            if (argSpec->Identifier() != nullptr) {
                auto label = argSpec->Identifier()->getText();

                auto token = argSpec->getStart();
                auto location = get_token_location(token);

                ArchetypeNode *keywordNode;
                TU_ASSIGN_OR_RAISE (keywordNode, state->appendNode(lyric_schema::kLyricAstKeywordClass, location));
                TU_RAISE_IF_NOT_OK (keywordNode->putAttr(kLyricAstIdentifier, label));
                TU_RAISE_IF_NOT_OK (keywordNode->appendChild(argNode));
                argNode = keywordNode;
            }

            argNodes.push_front(argNode);
        }

        ArchetypeNode *macroNode;
        TU_ASSIGN_OR_RAISE (macroNode, state->peekNode());
        for (auto *argNode : argNodes) {
            TU_RAISE_IF_NOT_OK (macroNode->appendChild(argNode));
        }
    }
}

void
lyric_parser::internal::ModuleMacroOps::enterMacroCall(ModuleParser::MacroCallContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *macroCallNode;
    TU_ASSIGN_OR_RAISE (macroCallNode, state->appendNode(lyric_schema::kLyricAstMacroCallClass, location));

    auto id = ctx->Identifier()->getText();
    TU_RAISE_IF_NOT_OK (macroCallNode->putAttr(kLyricAstIdentifier, id));

    TU_RAISE_IF_NOT_OK (state->pushNode(macroCallNode));
}

void
lyric_parser::internal::ModuleMacroOps::exitMacroCall(ModuleParser::MacroCallContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *macroCallNode;
    TU_ASSIGN_OR_RAISE (macroCallNode, state->popNode(lyric_schema::kLyricAstMacroCallClass));

    ArchetypeNode *macroListNode;
    TU_ASSIGN_OR_RAISE (macroListNode, state->peekNode(lyric_schema::kLyricAstMacroListClass));

    TU_RAISE_IF_NOT_OK (macroListNode->appendChild(macroCallNode));
}

void
lyric_parser::internal::ModuleMacroOps::enterMacroAnnotation(ModuleParser::MacroAnnotationContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *macroCallNode;
    TU_ASSIGN_OR_RAISE (macroCallNode, state->appendNode(lyric_schema::kLyricAstMacroCallClass, location));

    auto id = ctx->Identifier()->getText();
    TU_RAISE_IF_NOT_OK (macroCallNode->putAttr(kLyricAstIdentifier, id));

    TU_RAISE_IF_NOT_OK (state->pushNode(macroCallNode));
}

void
lyric_parser::internal::ModuleMacroOps::exitMacroAnnotation(ModuleParser::MacroAnnotationContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *macroCallNode;
    TU_ASSIGN_OR_RAISE (macroCallNode, state->popNode(lyric_schema::kLyricAstMacroCallClass));

    ArchetypeNode *macroListNode;
    TU_ASSIGN_OR_RAISE (macroListNode, state->peekNode(lyric_schema::kLyricAstMacroListClass));

    TU_RAISE_IF_NOT_OK (macroListNode->appendChild(macroCallNode));
}

void
lyric_parser::internal::ModuleMacroOps::enterPragmaMacro(ModuleParser::PragmaMacroContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *pragmaNode;
    TU_ASSIGN_OR_RAISE (pragmaNode, state->appendNode(lyric_schema::kLyricAstPragmaClass, location));

    auto id = ctx->Identifier()->getText();
    TU_RAISE_IF_NOT_OK (pragmaNode->putAttr(kLyricAstIdentifier, id));

    TU_RAISE_IF_NOT_OK (state->pushNode(pragmaNode));
}

void
lyric_parser::internal::ModuleMacroOps::exitPragmaMacro(ModuleParser::PragmaMacroContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *pragmaNode;
    TU_ASSIGN_OR_RAISE (pragmaNode, state->popNode(lyric_schema::kLyricAstPragmaClass));
    TU_RAISE_IF_NOT_OK (state->addPragma(pragmaNode));
}

void
lyric_parser::internal::ModuleMacroOps::enterDefinitionMacro(ModuleParser::DefinitionMacroContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *macroListNode;
    TU_ASSIGN_OR_RAISE (macroListNode, state->appendNode(lyric_schema::kLyricAstMacroListClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(macroListNode));
}

void
lyric_parser::internal::ModuleMacroOps::exitDefinitionMacro(ModuleParser::DefinitionMacroContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *macroListNode;
    TU_ASSIGN_OR_RAISE (macroListNode, state->popNode(lyric_schema::kLyricAstMacroListClass));

    ArchetypeNode *definitionNode;
    TU_ASSIGN_OR_RAISE (definitionNode, state->peekNode());
    TU_RAISE_IF_NOT_OK (definitionNode->putAttr(lyric_parser::kLyricAstMacroListOffset, macroListNode));
}

void
lyric_parser::internal::ModuleMacroOps::enterBlockMacro(ModuleParser::BlockMacroContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *macroListNode;
    TU_ASSIGN_OR_RAISE (macroListNode, state->appendNode(lyric_schema::kLyricAstMacroListClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(macroListNode));
}

void
lyric_parser::internal::ModuleMacroOps::exitBlockMacro(ModuleParser::BlockMacroContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    TU_RAISE_IF_STATUS (state->peekNode(lyric_schema::kLyricAstMacroListClass));
}
