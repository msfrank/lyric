
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_control_ops.h>
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
        m_state->throwIncompleteModule(ctx->getStop());
    auto *consequentNode = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *conditionNode = m_state->popNode();

    auto *token = ctx->getStart();

    auto *caseNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCaseClass, token);
    caseNode->appendChild(conditionNode);
    caseNode->appendChild(consequentNode);

    auto *ifNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstIfClass, token);
    ifNode->appendChild(caseNode);
    m_state->pushNode(ifNode);
}

void
lyric_parser::internal::ModuleControlOps::exitIfThenElseExpression(ModuleParser::IfThenElseExpressionContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *alternativeNode = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *consequentNode = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *conditionNode = m_state->popNode();

    auto *token = ctx->getStart();

    auto *caseNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCaseClass, token);
    caseNode->appendChild(conditionNode);
    caseNode->appendChild(consequentNode);

    auto *defaultOffsetAttr = m_state->appendAttrOrThrow(kLyricAstDefaultOffset,
        static_cast<tu_uint32>(alternativeNode->getAddress().getAddress()));

    auto *condNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCondClass, token);
    condNode->putAttr(defaultOffsetAttr);
    condNode->appendChild(caseNode);
    m_state->pushNode(condNode);
}

void
lyric_parser::internal::ModuleControlOps::enterCondExpression(ModuleParser::CondExpressionContext *ctx)
{
    auto *token = ctx->getStart();
    auto *condNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCondClass, token);
    m_state->pushNode(condNode);
}

void
lyric_parser::internal::ModuleControlOps::exitCondCase(ModuleParser::CondCaseContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *consequentNode = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *conditionNode = m_state->popNode();

    auto *token = ctx->getStart();

    auto *caseNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCaseClass, token);
    caseNode->appendChild(conditionNode);
    caseNode->appendChild(consequentNode);

    // if ancestor node is not a kCond, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *condNode = m_state->peekNode();
    condNode->checkClassOrThrow(lyric_schema::kLyricAstCondClass);

    // otherwise append case to the cond
    condNode->appendChild(caseNode);
}

void
lyric_parser::internal::ModuleControlOps::exitCondElse(ModuleParser::CondElseContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *alternativeNode = m_state->popNode();

    auto *defaultOffsetAttr = m_state->appendAttrOrThrow(kLyricAstDefaultOffset,
        static_cast<tu_uint32>(alternativeNode->getAddress().getAddress()));

    // if ancestor node is not a kCond, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *condNode = m_state->peekNode();
    condNode->checkClassOrThrow(lyric_schema::kLyricAstCondClass);

    // otherwise add defaultCase attribute to the cond
    condNode->putAttr(defaultOffsetAttr);
}

void
lyric_parser::internal::ModuleControlOps::enterCondIfStatement(ModuleParser::CondIfStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto *ifNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstIfClass, token);
    m_state->pushNode(ifNode);
}

void
lyric_parser::internal::ModuleControlOps::exitCondIfCase(ModuleParser::CondIfCaseContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *consequentNode = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *conditionNode = m_state->popNode();

    auto *token = ctx->getStart();

    auto *caseNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCaseClass, token);
    caseNode->appendChild(conditionNode);
    caseNode->appendChild(consequentNode);

    // if ancestor node is not a kIf, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *ifNode = m_state->peekNode();
    ifNode->checkClassOrThrow(lyric_schema::kLyricAstIfClass);

    // otherwise append case to the cond
    ifNode->appendChild(caseNode);
}

void
lyric_parser::internal::ModuleControlOps::exitCondIfElse(ModuleParser::CondIfElseContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *alternativeNode = m_state->popNode();

    auto *defaultOffsetAttr = m_state->appendAttrOrThrow(kLyricAstDefaultOffset,
        static_cast<tu_uint32>(alternativeNode->getAddress().getAddress()));

    // if ancestor node is not a kIf, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *ifNode = m_state->peekNode();
    ifNode->checkClassOrThrow(lyric_schema::kLyricAstIfClass);

    // otherwise add defaultCase attribute to the cond
    ifNode->putAttr(defaultOffsetAttr);
}

void
lyric_parser::internal::ModuleControlOps::enterWhileStatement(ModuleParser::WhileStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto *whileNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstWhileClass, token);
    m_state->pushNode(whileNode);
}

void
lyric_parser::internal::ModuleControlOps::exitWhileStatement(ModuleParser::WhileStatementContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *blockNode = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *testNode = m_state->popNode();

    // if ancestor node is not a kWhile, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *whileNode = m_state->peekNode();
    whileNode->checkClassOrThrow(lyric_schema::kLyricAstWhileClass);

    whileNode->appendChild(testNode);
    whileNode->appendChild(blockNode);
}

void
lyric_parser::internal::ModuleControlOps::enterForStatement(ModuleParser::ForStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto *forNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstForClass, token);
    m_state->pushNode(forNode);
}

void
lyric_parser::internal::ModuleControlOps::exitForStatement(ModuleParser::ForStatementContext * ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *blockNode = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *iteratorNode = m_state->popNode();

    auto id = ctx->Identifier()->getText();

    // if ancestor node is not a kFor, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *forNode = m_state->peekNode();
    forNode->checkClassOrThrow(lyric_schema::kLyricAstForClass);

    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);
    forNode->putAttr(identifierAttr);

    if (ctx->assignableType()) {
        auto *typeNode = m_state->makeType(ctx->assignableType());
        auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
            static_cast<tu_uint32>(typeNode->getAddress().getAddress()));
        forNode->putAttr(typeOffsetAttr);
    }

    forNode->appendChild(iteratorNode);
    forNode->appendChild(blockNode);
}

void
lyric_parser::internal::ModuleControlOps::exitReturnStatement(ModuleParser::ReturnStatementContext * ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *p1 = m_state->popNode();

    auto *token = ctx->getStart();

    auto *returnNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstReturnClass, token);
    returnNode->appendChild(p1);
    m_state->pushNode(returnNode);
}
