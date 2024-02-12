
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_deref_ops.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleDerefOps::ModuleDerefOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleDerefOps::enterThisExpression(ModuleParser::ThisExpressionContext *ctx)
{
    auto *token = ctx->getStart();
    auto *derefNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDerefClass, token);
    m_state->pushNode(derefNode);
}

void
lyric_parser::internal::ModuleDerefOps::enterNameExpression(ModuleParser::NameExpressionContext *ctx)
{
    auto *token = ctx->getStart();
    auto *derefNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDerefClass, token);
    m_state->pushNode(derefNode);
}

void
lyric_parser::internal::ModuleDerefOps::enterCallExpression(ModuleParser::CallExpressionContext *ctx)
{
    auto *token = ctx->getStart();
    auto *derefNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDerefClass, token);
    m_state->pushNode(derefNode);
}

void
lyric_parser::internal::ModuleDerefOps::exitThisSpec(ModuleParser::ThisSpecContext *ctx)
{
    auto *token = ctx->getStart();

    auto *thisNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstThisClass, token);

    // if ancestor node is not a kDeref, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *derefNode = m_state->peekNode();
    derefNode->checkClassOrThrow(lyric_schema::kLyricAstDerefClass);

    // otherwise append this to the deref
    derefNode->appendChild(thisNode);
}

void
lyric_parser::internal::ModuleDerefOps::exitNameSpec(ModuleParser::NameSpecContext *ctx)
{
    auto id = ctx->Identifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    auto *token = ctx->getStart();

    auto *nameNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstNameClass, token);
    nameNode->putAttr(identifierAttr);

    // if ancestor node is not a kDeref, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *derefNode = m_state->peekNode();
    derefNode->checkClassOrThrow(lyric_schema::kLyricAstDerefClass);

    // otherwise append name to the deref
    derefNode->appendChild(nameNode);
}

void
lyric_parser::internal::ModuleDerefOps::exitCallSpec(ModuleParser::CallSpecContext *ctx)
{
    auto *token = ctx->getStart();

    auto *callNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCallClass, token);

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

            callNode->prependChild(argNode);
        }
    }

    auto id = ctx->Identifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);
    callNode->putAttr(identifierAttr);

    // if ancestor node is not a kDeref, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *derefNode = m_state->peekNode();
    derefNode->checkClassOrThrow(lyric_schema::kLyricAstDerefClass);

    // otherwise append call to the deref
    derefNode->appendChild(callNode);
}

void
lyric_parser::internal::ModuleDerefOps::exitDerefMember(ModuleParser::DerefMemberContext *ctx)
{
    auto id = ctx->Identifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    auto *token = ctx->getStart();

    auto *nameNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstNameClass, token);
    nameNode->putAttr(identifierAttr);

    // if ancestor node is not a kDeref, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *derefNode = m_state->peekNode();
    derefNode->checkClassOrThrow(lyric_schema::kLyricAstDerefClass);

    // otherwise append name to the deref
    derefNode->appendChild(nameNode);
}

void
lyric_parser::internal::ModuleDerefOps::exitDerefMethod(ModuleParser::DerefMethodContext *ctx)
{
    auto *token = ctx->getStart();

    auto *callNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCallClass, token);

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

            callNode->prependChild(argNode);
        }
    }

    auto id = ctx->Identifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);
    callNode->putAttr(identifierAttr);

    // if ancestor node is not a kDeref, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *derefNode = m_state->peekNode();
    derefNode->checkClassOrThrow(lyric_schema::kLyricAstDerefClass);

    // otherwise append call to the deref
    derefNode->appendChild(callNode);
}

void
lyric_parser::internal::ModuleDerefOps::exitThisExpression(ModuleParser::ThisExpressionContext *ctx)
{
//    if (isEmpty() || peek()->getNodeType() != FirNodeType::FIR_DEREF)
//        throw ParseInvariantException(ctx->getStop(), "expected ancestor FIR_DEREF");
//    auto *deref = static_cast<FirDeref *>(peek());
//
//    // if deref only contains one element, then simplify the expression
//    if (deref->numP() == 1) {
//        auto *thiz = deref->getP(0)->clone();
//        delete pop();
//        push(thiz);
//    }
}

void
lyric_parser::internal::ModuleDerefOps::exitNameExpression(ModuleParser::NameExpressionContext *ctx)
{
//    if (isEmpty() || peek()->getNodeType() != FirNodeType::FIR_DEREF)
//        throw ParseInvariantException(ctx->getStop(), "expected ancestor FIR_DEREF");
//    auto *deref = static_cast<FirDeref *>(peek());
//
//    // if deref only contains one element, then simplify the expression
//    if (deref->numP() == 1) {
//        auto *name = deref->getP(0)->clone();
//        delete pop();
//        push(name);
//    }
}

void
lyric_parser::internal::ModuleDerefOps::exitCallExpression(ModuleParser::CallExpressionContext *ctx)
{
//    if (isEmpty() || peek()->getNodeType() != FirNodeType::FIR_DEREF)
//        throw ParseInvariantException(ctx->getStop(), "expected ancestor FIR_DEREF");
//    auto *deref = static_cast<FirDeref *>(peek());
//
//    // if deref only contains one element, then simplify the expression
//    if (deref->numP() == 1) {
//        auto *call = deref->getP(0)->clone();
//        delete pop();
//        push(call);
//    }
}
