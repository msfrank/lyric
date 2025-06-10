
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_definstance_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_attrs.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_tracing/enter_scope.h>
#include <tempo_tracing/exit_scope.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleDefinstanceOps::ModuleDefinstanceOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleDefinstanceOps::enterDefinstanceStatement(ModuleParser::DefinstanceStatementContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefinstanceOps::enterDefinstanceStatement");

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *definstanceNode;
    TU_ASSIGN_OR_RAISE (definstanceNode, m_state->appendNode(lyric_schema::kLyricAstDefInstanceClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(definstanceNode));

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);
}

void
lyric_parser::internal::ModuleDefinstanceOps::enterInstanceVal(ModuleParser::InstanceValContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefinstanceOps::enterInstanceVal");
}

void
lyric_parser::internal::ModuleDefinstanceOps::exitInstanceVal(ModuleParser::InstanceValContext *ctx)
{
    tempo_tracing::ExitScope scope;

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // member name
    auto id = ctx->symbolIdentifier()->getText();

    // member access level
    auto access = parse_access_type(id);

    // member type
    auto *memberTypeNode = make_Type_node(m_state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *valNode;
    TU_ASSIGN_OR_RAISE (valNode, m_state->appendNode(lyric_schema::kLyricAstValClass, location));
    TU_RAISE_IF_NOT_OK (valNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (valNode->putAttr(kLyricAstAccessType, access));
    TU_RAISE_IF_NOT_OK (valNode->putAttr(kLyricAstTypeOffset, memberTypeNode));

    // if member initializer is specified then set dfl
    if (ctx->defaultInitializer() != nullptr) {
        ArchetypeNode *defaultNode;
        TU_ASSIGN_OR_RAISE (defaultNode, m_state->popNode());
        TU_RAISE_IF_NOT_OK (valNode->appendChild(defaultNode));
    }

    ArchetypeNode *definstanceNode;
    TU_ASSIGN_OR_RAISE (definstanceNode, m_state->peekNode(lyric_schema::kLyricAstDefInstanceClass));

    TU_RAISE_IF_NOT_OK (definstanceNode->appendChild(valNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefinstanceOps::enterInstanceVar(ModuleParser::InstanceVarContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefinstanceOps::enterInstanceVar");
}

void
lyric_parser::internal::ModuleDefinstanceOps::exitInstanceVar(ModuleParser::InstanceVarContext *ctx)
{
    tempo_tracing::ExitScope scope;

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // member name
    auto id = ctx->symbolIdentifier()->getText();

    // member access level
    auto access = parse_access_type(id);

    // member type
    auto *memberTypeNode = make_Type_node(m_state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *varNode;
    TU_ASSIGN_OR_RAISE (varNode, m_state->appendNode(lyric_schema::kLyricAstVarClass, location));
    TU_RAISE_IF_NOT_OK (varNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (varNode->putAttr(kLyricAstAccessType, access));
    TU_RAISE_IF_NOT_OK (varNode->putAttr(kLyricAstTypeOffset, memberTypeNode));

    // if member initializer is specified then set dfl
    if (ctx->defaultInitializer() != nullptr) {
        ArchetypeNode *defaultNode;
        TU_ASSIGN_OR_RAISE (defaultNode, m_state->popNode());
        TU_RAISE_IF_NOT_OK (varNode->appendChild(defaultNode));
    }

    ArchetypeNode *definstanceNode;
    TU_ASSIGN_OR_RAISE (definstanceNode, m_state->peekNode(lyric_schema::kLyricAstDefInstanceClass));

    TU_RAISE_IF_NOT_OK (definstanceNode->appendChild(varNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefinstanceOps::enterInstanceDef(ModuleParser::InstanceDefContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefinstanceOps::enterInstanceDef");
}

void
lyric_parser::internal::ModuleDefinstanceOps::exitInstanceDef(ModuleParser::InstanceDefContext *ctx)
{
    tempo_tracing::ExitScope scope;

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    ArchetypeNode *blockNode;
    TU_ASSIGN_OR_RAISE (blockNode, m_state->popNode());

    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *defNode;
    TU_ASSIGN_OR_RAISE (defNode, m_state->appendNode(lyric_schema::kLyricAstDefClass, location));
    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    // the method name
    auto id = ctx->symbolIdentifier()->getText();
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstIdentifier, id));

    // the visibility
    auto access = parse_access_type(id);
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstAccessType, access));

    // the method return type
    if (ctx->returnSpec()) {
        auto *returnTypeNode = make_Type_node(m_state, ctx->returnSpec()->assignableType());
        TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstTypeOffset, returnTypeNode));
    } else {
        ArchetypeNode *returnTypeNode;
        TU_ASSIGN_OR_RAISE (returnTypeNode, m_state->appendNode(lyric_schema::kLyricAstXTypeClass, location));
        TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstTypeOffset, returnTypeNode));
    }

    TU_RAISE_IF_NOT_OK (defNode->appendChild(packNode));
    TU_RAISE_IF_NOT_OK (defNode->appendChild(blockNode));

    ArchetypeNode *definstanceNode;
    TU_ASSIGN_OR_RAISE (definstanceNode, m_state->peekNode(lyric_schema::kLyricAstDefInstanceClass));

    TU_RAISE_IF_NOT_OK (definstanceNode->appendChild(defNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefinstanceOps::enterInstanceImpl(ModuleParser::InstanceImplContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefinstanceOps::enterInstanceImpl");

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *implNode;
    TU_ASSIGN_OR_RAISE (implNode, m_state->appendNode(lyric_schema::kLyricAstImplClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(implNode));

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);
}

void
lyric_parser::internal::ModuleDefinstanceOps::exitInstanceImpl(ModuleParser::InstanceImplContext *ctx)
{
    tempo_tracing::ExitScope scope;

    // the impl type
    auto *implTypeNode = make_Type_node(m_state, ctx->assignableType());

    ArchetypeNode *implNode;
    TU_ASSIGN_OR_RAISE (implNode, m_state->popNode(lyric_schema::kLyricAstImplClass));

    // set the impl type
    TU_RAISE_IF_NOT_OK (implNode->putAttr(kLyricAstTypeOffset, implTypeNode));

    ArchetypeNode *definstanceNode;
    TU_ASSIGN_OR_RAISE (definstanceNode, m_state->peekNode(lyric_schema::kLyricAstDefInstanceClass));

    TU_RAISE_IF_NOT_OK (definstanceNode->appendChild(implNode));
}

void
lyric_parser::internal::ModuleDefinstanceOps::exitDefinstanceStatement(ModuleParser::DefinstanceStatementContext *ctx)
{
    tempo_tracing::ExitScope scope;

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // the instance name
    auto id = ctx->symbolIdentifier()->getText();

    // the instance access level
    auto access = parse_access_type(id);

    // the class derive type
    DeriveType derive = DeriveType::Any;
    if (ctx->instanceDerives()) {
        if (ctx->instanceDerives()->SealedKeyword() != nullptr) {
            derive = DeriveType::Sealed;
        } else if (ctx->instanceDerives()->FinalKeyword() != nullptr) {
            derive = DeriveType::Final;
        }
    }

    ArchetypeNode *definstanceNode;
    TU_ASSIGN_OR_RAISE (definstanceNode, m_state->peekNode(lyric_schema::kLyricAstDefInstanceClass));

    TU_RAISE_IF_NOT_OK (definstanceNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (definstanceNode->putAttr(kLyricAstAccessType, access));
    TU_RAISE_IF_NOT_OK (definstanceNode->putAttr(kLyricAstDeriveType, derive));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}