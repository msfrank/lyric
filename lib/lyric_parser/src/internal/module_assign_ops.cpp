
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_assign_ops.h>
#include <lyric_parser/internal/parser_utils.h>
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
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *p1 = m_state->popNode();

    auto id = ctx->symbolIdentifier()->getText();
    auto access = parse_access_type(id);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *valNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstValClass, location);
    valNode->appendChild(p1);
    valNode->putAttrOrThrow(kLyricAstIdentifier, id);
    valNode->putAttrOrThrow(kLyricAstAccessType, access);
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
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *p1 = m_state->popNode();

    auto id = ctx->symbolIdentifier()->getText();
    auto access = parse_access_type(id);
    auto *typeNode = make_Type_node(m_state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *valNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstValClass, location);
    valNode->appendChild(p1);
    valNode->putAttr(kLyricAstIdentifier, id);
    valNode->putAttrOrThrow(kLyricAstAccessType, access);
    valNode->putAttr(kLyricAstTypeOffset, typeNode);
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
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *p1 = m_state->popNode();

    auto id = ctx->symbolIdentifier()->getText();
    auto access = parse_access_type(id);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *varNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstVarClass, location);
    varNode->appendChild(p1);
    varNode->putAttr(kLyricAstIdentifier, id);
    varNode->putAttrOrThrow(kLyricAstAccessType, access);
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
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *p1 = m_state->popNode();

    auto id = ctx->symbolIdentifier()->getText();
    auto access = parse_access_type(id);
    auto *typeNode = make_Type_node(m_state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *varNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstVarClass, location);
    varNode->appendChild(p1);
    varNode->putAttr(kLyricAstIdentifier, id);
    varNode->putAttrOrThrow(kLyricAstAccessType, access);
    varNode->putAttr(kLyricAstTypeOffset, typeNode);
    m_state->pushNode(varNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleAssignOps::exitNameAssignment(ModuleParser::NameAssignmentContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto id = ctx->Identifier()->getText();

    auto *nameNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstNameClass, location);
    nameNode->putAttr(kLyricAstIdentifier, id);
    m_state->pushNode(nameNode);
}

void
lyric_parser::internal::ModuleAssignOps::exitMemberAssignment(ModuleParser::MemberAssignmentContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *targetNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstTargetClass, location);

    if (ctx->ThisKeyword()) {
        token = ctx->ThisKeyword()->getSymbol();
        location = get_token_location(token);
        auto *thisNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstThisClass, location);
        targetNode->appendChild(thisNode);
    }

    for (size_t i = 0; i < ctx->getRuleIndex(); i++) {
        if (ctx->Identifier(i) == nullptr)
            continue;
        token = ctx->Identifier(i)->getSymbol();
        location = get_token_location(token);

        auto id = ctx->Identifier(i)->getText();

        auto *nameNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstNameClass, location);
        nameNode->putAttr(kLyricAstIdentifier, id);
        targetNode->appendChild(nameNode);
    }

    m_state->pushNode(targetNode);
}

void
lyric_parser::internal::ModuleAssignOps::exitSetStatement(ModuleParser::SetStatementContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *exprNode = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *targetNode = m_state->popNode();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *setNode = nullptr;
    if (ctx->assignmentOp()->AssignOperator()) {
        setNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstSetClass, location);
    } else if (ctx->assignmentOp()->PlusAssignOperator()) {
        setNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstInplaceAddClass, location);
    } else if (ctx->assignmentOp()->MinusAssignOperator()) {
        setNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstInplaceSubClass, location);
    } else if (ctx->assignmentOp()->StarAssignOperator()) {
        setNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstInplaceMulClass, location);
    } else if (ctx->assignmentOp()->SlashAssignOperator()) {
        setNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstInplaceDivClass, location);
    } else {
        // FIXME: this should be a parse invariant exception
        m_state->throwSyntaxError(location, "illegal set operator");
    }

    setNode->appendChild(targetNode);
    setNode->appendChild(exprNode);
    m_state->pushNode(setNode);
}