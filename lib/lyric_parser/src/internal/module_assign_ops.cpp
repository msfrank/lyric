
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_assign_ops.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleAssignOps::ModuleAssignOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleAssignOps::enterUntypedVal(ModuleParser::UntypedValContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleAssignOps::exitUntypedVal(ModuleParser::UntypedValContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();
    span->putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *p1 = m_state->popNode();

    auto id = ctx->symbolIdentifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    auto *token = ctx->getStart();

    auto *valNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstValClass, token);
    valNode->appendChild(p1);
    valNode->putAttr(identifierAttr);
    m_state->pushNode(valNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleAssignOps::enterTypedVal(ModuleParser::TypedValContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleAssignOps::exitTypedVal(ModuleParser::TypedValContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();
    span->putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *p1 = m_state->popNode();

    auto id = ctx->symbolIdentifier()->getText();

    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    auto *typeNode = m_state->makeType(ctx->assignableType());
    auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
        static_cast<tu_uint32>(typeNode->getAddress().getAddress()));

    auto *token = ctx->getStart();

    auto *valNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstValClass, token);
    valNode->appendChild(p1);
    valNode->putAttr(identifierAttr);
    valNode->putAttr(typeOffsetAttr);
    m_state->pushNode(valNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleAssignOps::enterUntypedVar(ModuleParser::UntypedVarContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleAssignOps::exitUntypedVar(ModuleParser::UntypedVarContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();
    span->putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *p1 = m_state->popNode();

    auto id = ctx->symbolIdentifier()->getText();

    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    auto *token = ctx->getStart();

    auto *varNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstVarClass, token);
    varNode->appendChild(p1);
    varNode->putAttr(identifierAttr);
    m_state->pushNode(varNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleAssignOps::enterTypedVar(ModuleParser::TypedVarContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleAssignOps::exitTypedVar(ModuleParser::TypedVarContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();
    span->putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *p1 = m_state->popNode();

    auto id = ctx->symbolIdentifier()->getText();

    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    auto *typeNode = m_state->makeType(ctx->assignableType());
    auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
        static_cast<tu_uint32>(typeNode->getAddress().getAddress()));

    auto *token = ctx->getStart();

    auto *varNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstVarClass, token);
    varNode->appendChild(p1);
    varNode->putAttr(identifierAttr);
    varNode->putAttr(typeOffsetAttr);
    m_state->pushNode(varNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleAssignOps::exitNameAssignment(ModuleParser::NameAssignmentContext *ctx)
{
    auto *token = ctx->getStart();

    auto id = ctx->Identifier()->getText();

    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);
    auto *nameNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstNameClass, token);
    nameNode->putAttr(identifierAttr);
    m_state->pushNode(nameNode);
}

void
lyric_parser::internal::ModuleAssignOps::exitMemberAssignment(ModuleParser::MemberAssignmentContext *ctx)
{
    auto *token = ctx->getStart();

    auto *targetNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstTargetClass, token);

    if (ctx->ThisKeyword()) {
        token = ctx->ThisKeyword()->getSymbol();
        auto *thisNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstThisClass, token);
        targetNode->appendChild(thisNode);
    }

    for (size_t i = 0; i < ctx->getRuleIndex(); i++) {
        if (ctx->Identifier(i) == nullptr)
            continue;
        token = ctx->Identifier(i)->getSymbol();

        auto id = ctx->Identifier(i)->getText();

        auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);
        auto *nameNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstNameClass, token);
        nameNode->putAttr(identifierAttr);
        targetNode->appendChild(nameNode);
    }

    m_state->pushNode(targetNode);
}

void
lyric_parser::internal::ModuleAssignOps::exitSetStatement(ModuleParser::SetStatementContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *exprNode = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *targetNode = m_state->popNode();

    auto *token = ctx->getStart();

    ArchetypeNode *setNode = nullptr;
    if (ctx->assignmentOp()->AssignOperator()) {
        setNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstSetClass, token);
    } else if (ctx->assignmentOp()->PlusAssignOperator()) {
        setNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstInplaceAddClass, token);
    } else if (ctx->assignmentOp()->MinusAssignOperator()) {
        setNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstInplaceSubClass, token);
    } else if (ctx->assignmentOp()->StarAssignOperator()) {
        setNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstInplaceMulClass, token);
    } else if (ctx->assignmentOp()->SlashAssignOperator()) {
        setNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstInplaceDivClass, token);
    } else {
        // FIXME: this should be a parse invariant exception
        m_state->throwSyntaxError(ctx->getStart(), "illegal set operator");
    }

    setNode->appendChild(targetNode);
    setNode->appendChild(exprNode);
    m_state->pushNode(setNode);
}