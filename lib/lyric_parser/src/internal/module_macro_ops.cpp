
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
    if (ctx->argList()) {
        auto *argList = ctx->argList();

        std::deque<ArchetypeNode *> argNodes;
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argSpec(i);
            if (argSpec == nullptr)
                continue;

            ArchetypeNode *argNode;
            TU_ASSIGN_OR_RAISE (argNode, m_state->popNode());

            if (argSpec->Identifier() != nullptr) {
                auto label = argSpec->Identifier()->getText();

                auto token = argSpec->getStart();
                auto location = get_token_location(token);

                ArchetypeNode *keywordNode;
                TU_ASSIGN_OR_RAISE (keywordNode, m_state->appendNode(lyric_schema::kLyricAstKeywordClass, location));
                TU_RAISE_IF_NOT_OK (keywordNode->putAttr(kLyricAstIdentifier, label));
                TU_RAISE_IF_NOT_OK (keywordNode->appendChild(argNode));
                argNode = keywordNode;
            }

            argNodes.push_front(argNode);
        }

        ArchetypeNode *macroNode;
        TU_ASSIGN_OR_RAISE (macroNode, m_state->peekNode());
        for (auto *argNode : argNodes) {
            TU_RAISE_IF_NOT_OK (macroNode->appendChild(argNode));
        }
    }
}

void
lyric_parser::internal::ModuleMacroOps::enterMacroCall(ModuleParser::MacroCallContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *macroCallNode;
    TU_ASSIGN_OR_RAISE (macroCallNode, m_state->appendNode(lyric_schema::kLyricAstMacroCallClass, location));

    auto id = ctx->Identifier()->getText();
    TU_RAISE_IF_NOT_OK (macroCallNode->putAttr(kLyricAstIdentifier, id));

    TU_RAISE_IF_NOT_OK (m_state->pushNode(macroCallNode));
}

void
lyric_parser::internal::ModuleMacroOps::exitMacroCall(ModuleParser::MacroCallContext *ctx)
{
    ArchetypeNode *macroCallNode;
    TU_ASSIGN_OR_RAISE (macroCallNode, m_state->popNode(lyric_schema::kLyricAstMacroCallClass));

    // if ancestor node is not a kMacroList, then report internal violation
    ArchetypeNode *macroListNode;
    TU_ASSIGN_OR_RAISE (macroListNode, m_state->peekNode(lyric_schema::kLyricAstMacroListClass));

    // otherwise append call to the macro list
    TU_RAISE_IF_NOT_OK (macroListNode->appendChild(macroCallNode));
}

void
lyric_parser::internal::ModuleMacroOps::enterMacroAnnotation(ModuleParser::MacroAnnotationContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *macroCallNode;
    TU_ASSIGN_OR_RAISE (macroCallNode, m_state->appendNode(lyric_schema::kLyricAstMacroCallClass, location));

    auto id = ctx->Identifier()->getText();
    TU_RAISE_IF_NOT_OK (macroCallNode->putAttr(kLyricAstIdentifier, id));

    TU_RAISE_IF_NOT_OK (m_state->pushNode(macroCallNode));
}

void
lyric_parser::internal::ModuleMacroOps::exitMacroAnnotation(ModuleParser::MacroAnnotationContext *ctx)
{
    ArchetypeNode *macroCallNode;
    TU_ASSIGN_OR_RAISE (macroCallNode, m_state->popNode(lyric_schema::kLyricAstMacroCallClass));

    // if ancestor node is not a kMacroList, then report internal violation
    ArchetypeNode *macroListNode;
    TU_ASSIGN_OR_RAISE (macroListNode, m_state->peekNode(lyric_schema::kLyricAstMacroListClass));

    // otherwise append call to the macro list
    TU_RAISE_IF_NOT_OK (macroListNode->appendChild(macroCallNode));
}

void
lyric_parser::internal::ModuleMacroOps::enterPragmaMacro(ModuleParser::PragmaMacroContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *pragmaNode;
    TU_ASSIGN_OR_RAISE (pragmaNode, m_state->appendNode(lyric_schema::kLyricAstPragmaClass, location));

    auto id = ctx->Identifier()->getText();
    TU_RAISE_IF_NOT_OK (pragmaNode->putAttr(kLyricAstIdentifier, id));

    TU_RAISE_IF_NOT_OK (m_state->pushNode(pragmaNode));
}

void
lyric_parser::internal::ModuleMacroOps::exitPragmaMacro(ModuleParser::PragmaMacroContext *ctx)
{
    ArchetypeNode *pragmaNode;
    TU_ASSIGN_OR_RAISE (pragmaNode, m_state->popNode(lyric_schema::kLyricAstPragmaClass));
    TU_RAISE_IF_NOT_OK (m_state->addPragma(pragmaNode));
}

void
lyric_parser::internal::ModuleMacroOps::enterDefinitionMacro(ModuleParser::DefinitionMacroContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *macroListNode;
    TU_ASSIGN_OR_RAISE (macroListNode, m_state->appendNode(lyric_schema::kLyricAstMacroListClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(macroListNode));
}

void
lyric_parser::internal::ModuleMacroOps::exitDefinitionMacro(ModuleParser::DefinitionMacroContext *ctx)
{
    ArchetypeNode *macroListNode;
    TU_ASSIGN_OR_RAISE (macroListNode, m_state->popNode(lyric_schema::kLyricAstMacroListClass));

    ArchetypeNode *definitionNode;
    TU_ASSIGN_OR_RAISE (definitionNode, m_state->peekNode());
    TU_RAISE_IF_NOT_OK (definitionNode->putAttr(lyric_parser::kLyricAstMacroListOffset, macroListNode));
}

void
lyric_parser::internal::ModuleMacroOps::enterBlockMacro(ModuleParser::BlockMacroContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *macroListNode;
    TU_ASSIGN_OR_RAISE (macroListNode, m_state->appendNode(lyric_schema::kLyricAstMacroListClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(macroListNode));
}

void
lyric_parser::internal::ModuleMacroOps::exitBlockMacro(ModuleParser::BlockMacroContext *ctx)
{
    TU_RAISE_IF_STATUS (m_state->peekNode(lyric_schema::kLyricAstMacroListClass));
}
