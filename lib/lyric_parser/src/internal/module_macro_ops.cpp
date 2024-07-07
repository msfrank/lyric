
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_macro_ops.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleMacroOps::ModuleMacroOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleMacroOps::enterMacro(ModuleParser::MacroContext *ctx)
{
    auto *token = ctx->getStart();
    auto *macroListNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstMacroListClass, token);
    m_state->pushNode(macroListNode);
}

void
lyric_parser::internal::ModuleMacroOps::enterAnnotationList(ModuleParser::AnnotationListContext *ctx)
{
    auto *token = ctx->getStart();
    auto *macroListNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstMacroListClass, token);
    m_state->pushNode(macroListNode);
}

void
lyric_parser::internal::ModuleMacroOps::exitMacroCall(ModuleParser::MacroCallContext *ctx)
{
    auto *token = ctx->getStart();

    auto *macroCallNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstMacroCallClass, token);

    if (ctx->argList()) {
        auto *argList = ctx->argList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argSpec(i);
            if (argSpec == nullptr)
                continue;

            if (m_state->isEmpty())
                m_state->throwIncompleteModule(ctx->getStop());
            auto *argNode = m_state->popNode();

            if (argSpec->Identifier() != nullptr) {
                auto label = argSpec->Identifier()->getText();
                auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, label);

                token = argSpec->getStart();

                auto *keywordNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstKeywordClass, token);
                keywordNode->putAttr(identifierAttr);
                keywordNode->appendChild(argNode);
                argNode = keywordNode;
            }

            macroCallNode->prependChild(argNode);
        }
    }

    auto id = ctx->Identifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);
    macroCallNode->putAttr(identifierAttr);

    // if ancestor node is not a kMacroList, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *macroListNode = m_state->peekNode();
    macroListNode->checkClassOrThrow(lyric_schema::kLyricAstMacroListClass);

    // otherwise append call to the macro list
    macroListNode->appendChild(macroCallNode);
}

void
lyric_parser::internal::ModuleMacroOps::exitAnnotationList(ModuleParser::AnnotationListContext *ctx)
{
}

void
lyric_parser::internal::ModuleMacroOps::exitMacro(ModuleParser::MacroContext *ctx)
{
}
