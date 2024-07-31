
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_control_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleControlOps::ModuleControlOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleControlOps::exitIfStatement(ModuleParser::IfStatementContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *consequentNode = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *conditionNode = m_state->popNode();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *caseNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCaseClass, location);
    caseNode->appendChild(conditionNode);
    caseNode->appendChild(consequentNode);

    auto *ifNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstIfClass, location);
    ifNode->appendChild(caseNode);
    m_state->pushNode(ifNode);
}

void
lyric_parser::internal::ModuleControlOps::exitIfThenElseExpression(ModuleParser::IfThenElseExpressionContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *alternativeNode = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *consequentNode = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *conditionNode = m_state->popNode();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *caseNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCaseClass, location);
    caseNode->appendChild(conditionNode);
    caseNode->appendChild(consequentNode);

    auto *condNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCondClass, location);
    condNode->putAttr(kLyricAstDefaultOffset, alternativeNode);
    condNode->appendChild(caseNode);
    m_state->pushNode(condNode);
}

void
lyric_parser::internal::ModuleControlOps::enterCondExpression(ModuleParser::CondExpressionContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *condNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCondClass, location);
    m_state->pushNode(condNode);
}

void
lyric_parser::internal::ModuleControlOps::exitCondCase(ModuleParser::CondCaseContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *consequentNode = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *conditionNode = m_state->popNode();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *caseNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCaseClass, location);
    caseNode->appendChild(conditionNode);
    caseNode->appendChild(consequentNode);

    // if ancestor node is not a kCond, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *condNode = m_state->peekNode();
    m_state->checkNodeOrThrow(condNode, lyric_schema::kLyricAstCondClass);

    // otherwise append case to the cond
    condNode->appendChild(caseNode);
}

void
lyric_parser::internal::ModuleControlOps::exitCondElse(ModuleParser::CondElseContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *alternativeNode = m_state->popNode();

    // if ancestor node is not a kCond, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *condNode = m_state->peekNode();
    m_state->checkNodeOrThrow(condNode, lyric_schema::kLyricAstCondClass);

    // otherwise add defaultCase attribute to the cond
    condNode->putAttr(kLyricAstDefaultOffset, alternativeNode);
}

void
lyric_parser::internal::ModuleControlOps::enterCondIfStatement(ModuleParser::CondIfStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *ifNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstIfClass, location);
    m_state->pushNode(ifNode);
}

void
lyric_parser::internal::ModuleControlOps::exitCondIfCase(ModuleParser::CondIfCaseContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *consequentNode = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *conditionNode = m_state->popNode();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *caseNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCaseClass, location);
    caseNode->appendChild(conditionNode);
    caseNode->appendChild(consequentNode);

    // if ancestor node is not a kIf, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *ifNode = m_state->peekNode();
    m_state->checkNodeOrThrow(ifNode, lyric_schema::kLyricAstIfClass);

    // otherwise append case to the cond
    ifNode->appendChild(caseNode);
}

void
lyric_parser::internal::ModuleControlOps::exitCondIfElse(ModuleParser::CondIfElseContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *alternativeNode = m_state->popNode();

    // if ancestor node is not a kIf, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *ifNode = m_state->peekNode();
    m_state->checkNodeOrThrow(ifNode, lyric_schema::kLyricAstIfClass);

    // otherwise add defaultCase attribute to the cond
    ifNode->putAttr(kLyricAstDefaultOffset, alternativeNode);
}

void
lyric_parser::internal::ModuleControlOps::enterWhileStatement(ModuleParser::WhileStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *whileNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstWhileClass, location);
    m_state->pushNode(whileNode);
}

void
lyric_parser::internal::ModuleControlOps::exitWhileStatement(ModuleParser::WhileStatementContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *blockNode = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *testNode = m_state->popNode();

    // if ancestor node is not a kWhile, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *whileNode = m_state->peekNode();
    m_state->checkNodeOrThrow(whileNode, lyric_schema::kLyricAstWhileClass);

    whileNode->appendChild(testNode);
    whileNode->appendChild(blockNode);
}

void
lyric_parser::internal::ModuleControlOps::enterForStatement(ModuleParser::ForStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *forNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstForClass, location);
    m_state->pushNode(forNode);
}

void
lyric_parser::internal::ModuleControlOps::exitForStatement(ModuleParser::ForStatementContext * ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *blockNode = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *iteratorNode = m_state->popNode();

    auto id = ctx->Identifier()->getText();

    // if ancestor node is not a kFor, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *forNode = m_state->peekNode();
    m_state->checkNodeOrThrow(forNode, lyric_schema::kLyricAstForClass);

    forNode->putAttr(kLyricAstIdentifier, id);

    if (ctx->assignableType()) {
        auto *typeNode = make_Type_node(m_state, ctx->assignableType());
        forNode->putAttr(kLyricAstTypeOffset, typeNode);
    }

    forNode->appendChild(iteratorNode);
    forNode->appendChild(blockNode);
}

void
lyric_parser::internal::ModuleControlOps::exitReturnStatement(ModuleParser::ReturnStatementContext * ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *p1 = m_state->popNode();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *returnNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstReturnClass, location);
    returnNode->appendChild(p1);
    m_state->pushNode(returnNode);
}
