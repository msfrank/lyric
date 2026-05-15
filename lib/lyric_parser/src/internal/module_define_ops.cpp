
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_define_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_attrs.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_tracing/enter_scope.h>
#include <tempo_tracing/exit_scope.h>
#include <tempo_tracing/leaf_scope.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleDefineOps::ModuleDefineOps(ModuleArchetype *listener)
    : BaseOps(listener)
{
}

void
lyric_parser::internal::ModuleDefineOps::exitTypenameStatement(ModuleParser::TypenameStatementContext *ctx)
{
    tempo_tracing::LeafScope scope("lyric_parser::internal::ModuleDefineOps::exitTypenameStatement");

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    // pop the top of the symbol stack and verify that the identifier matches
    auto id = ctx->symbolIdentifier()->getText();
    state->popSymbolAndCheck(id);

    auto isHidden = identifier_is_hidden(id);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    if (hasError())
        return;

    ArchetypeNode *typenameNode;
    TU_ASSIGN_OR_RAISE (typenameNode, state->appendNode(lyric_schema::kLyricAstTypeNameClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(typenameNode));

    TU_RAISE_IF_NOT_OK (typenameNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (typenameNode->putAttr(kLyricAstIsHidden, isHidden));
}

void
lyric_parser::internal::ModuleDefineOps::enterDefStatement(ModuleParser::DefStatementContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefineOps::enterDefStatement");
}

void
lyric_parser::internal::ModuleDefineOps::exitDefStatement(ModuleParser::DefStatementContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    // pop the top of the symbol stack and verify that the identifier matches
    auto id = ctx->symbolIdentifier()->getText();
    state->popSymbolAndCheck(id);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto isHidden = identifier_is_hidden(id);

    if (hasError())
        return;

    // if def statement has a block then block is top of the stack, otherwise synthesize an empty node
    ArchetypeNode *blockNode;
    if (ctx->procBlock() && ctx->procBlock()->block()) {
        TU_ASSIGN_OR_RAISE (blockNode, state->popNode());
    } else {
        TU_ASSIGN_OR_RAISE (blockNode, state->appendNode(lyric_schema::kLyricAstBlockClass, {}));
    }

    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->popNode());

    // allocate the node
    ArchetypeNode *defNode;
    TU_ASSIGN_OR_RAISE (defNode, state->appendNode(lyric_schema::kLyricAstDefClass, location));
    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    // the function name
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstIdentifier, id));

    // the function visibility
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstIsHidden, isHidden));

    // the return type
    if (ctx->returnSpec()) {
        auto *returnTypeNode = make_Type_node(state, ctx->returnSpec()->assignableType());
        TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstTypeOffset, returnTypeNode));
    } else {
        ArchetypeNode *returnTypeNode;
        TU_ASSIGN_OR_RAISE (returnTypeNode, state->appendNode(lyric_schema::kLyricAstXTypeClass, location));
        TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstTypeOffset, returnTypeNode));
    }

    // generic information
    if (ctx->placeholderSpec()) {
        auto *genericNode = make_Generic_node(state, ctx->placeholderSpec(), ctx->constraintSpec());
        TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstGenericOffset, genericNode));
    }

    TU_RAISE_IF_NOT_OK (defNode->appendChild(packNode));
    TU_RAISE_IF_NOT_OK (defNode->appendChild(blockNode));

    TU_RAISE_IF_NOT_OK (state->pushNode(defNode));
}

void
lyric_parser::internal::ModuleDefineOps::enterImplDef(ModuleParser::ImplDefContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefineOps::enterImplDef");
}

void
lyric_parser::internal::ModuleDefineOps::exitImplDef(ModuleParser::ImplDefContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    // pop the top of the symbol stack and verify that the identifier matches
    auto id = ctx->symbolIdentifier()->getText();
    state->popSymbolAndCheck(id);

    auto isHidden = identifier_is_hidden(id);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    if (hasError())
        return;

    // if def statement has a block then block is top of the stack, otherwise synthesize an empty node
    ArchetypeNode *blockNode;
    if (ctx->procBlock() && ctx->procBlock()->block()) {
        TU_ASSIGN_OR_RAISE (blockNode, state->popNode());
    } else {
        TU_ASSIGN_OR_RAISE (blockNode, state->appendNode(lyric_schema::kLyricAstBlockClass, {}));
    }

    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->popNode());

    ArchetypeNode *defNode;
    TU_ASSIGN_OR_RAISE (defNode, state->appendNode(lyric_schema::kLyricAstDefClass, location));
    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    // the function name
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstIdentifier, id));

    // the function visibility
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstIsHidden, isHidden));

    // the return type
    if (ctx->returnSpec()) {
        auto *returnTypeNode = make_Type_node(state, ctx->returnSpec()->assignableType());
        TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstTypeOffset, returnTypeNode));
    } else {
        ArchetypeNode *returnTypeNode;
        TU_ASSIGN_OR_RAISE (returnTypeNode, state->appendNode(lyric_schema::kLyricAstXTypeClass, location));
        TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstTypeOffset, returnTypeNode));
    }

    // append pack node to the def
    TU_RAISE_IF_NOT_OK (defNode->appendChild(packNode));

    // append block node to the def
    TU_RAISE_IF_NOT_OK (defNode->appendChild(blockNode));

    // peek node on stack, verify it is impl
    ArchetypeNode *implNode;
    TU_ASSIGN_OR_RAISE (implNode, state->peekNode(lyric_schema::kLyricAstImplClass));

    // append def node to impl
    TU_RAISE_IF_NOT_OK (implNode->appendChild(defNode));
}

void
lyric_parser::internal::ModuleDefineOps::enterImplExt(ModuleParser::ImplExtContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefineOps::enterImplExt");
}

void
lyric_parser::internal::ModuleDefineOps::exitImplExt(ModuleParser::ImplExtContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();

    if (hasError())
        return;

    ArchetypeNode *extNode;
    TU_ASSIGN_OR_RAISE (extNode, state->popNode());

    lyric_schema::LyricAstId astId;
    TU_RAISE_IF_NOT_OK (extNode->parseId(lyric_schema::kLyricAstVocabulary, astId));
    switch (astId) {
        case lyric_schema::LyricAstId::Alias:
        case lyric_schema::LyricAstId::DefClass:
        case lyric_schema::LyricAstId::DefConcept:
        case lyric_schema::LyricAstId::DefEnum:
        case lyric_schema::LyricAstId::DefInstance:
        case lyric_schema::LyricAstId::DefStruct:
            break;
        default:
            throw tempo_utils::StatusException(ParseStatus::forCondition(
                ParseCondition::kParseInvariant, "invalid external definition"));
    }

    // peek node on stack, verify it is impl
    ArchetypeNode *implNode;
    TU_ASSIGN_OR_RAISE (implNode, state->peekNode(lyric_schema::kLyricAstImplClass));

    // append ext node to impl
    TU_RAISE_IF_NOT_OK (implNode->appendChild(extNode));
}

void
lyric_parser::internal::ModuleDefineOps::parseModifierSpec(ModuleParser::ModifierSpecContext *ctx)
{
    auto *state = getState();

    // peek node on stack
    ArchetypeNode *definitionNode;
    TU_ASSIGN_OR_RAISE (definitionNode, state->peekNode());

    bool isAbstract;
    if (definitionNode->hasAttr(kLyricAstIsAbstract)) {
        TU_RAISE_IF_NOT_OK (definitionNode->parseAttr(lyric_parser::kLyricAstIsAbstract, isAbstract));
    } else {
        isAbstract = false;
    }

    DeriveType derive;
    if (definitionNode->hasAttr(kLyricAstDeriveType)) {
        TU_RAISE_IF_NOT_OK (definitionNode->parseAttr(lyric_parser::kLyricAstDeriveType, derive));
    } else {
        derive = DeriveType::Any;
    }

    auto currentSymbol = state->currentSymbolString();

    if (ctx->abstractModifier()) {
        if (isAbstract) {
            logErrorOrThrow(ctx->getStart(), "{} is already declared abstract", currentSymbol);
            return;
        }
        if (derive == DeriveType::Final) {
            logErrorOrThrow(ctx->getStart(), "{} cannot be declared final and abstract", currentSymbol);
            return;
        }
        TU_RAISE_IF_NOT_OK (definitionNode->putAttr(kLyricAstIsAbstract, true));
        return;
    }

    if (ctx->sealedModifier()) {
        switch (derive) {
            case DeriveType::Final:
                logErrorOrThrow(ctx->getStart(), "{} is already declared final", currentSymbol);
                return;
            case DeriveType::Sealed:
                logErrorOrThrow(ctx->getStart(), "{} is already declared sealed", currentSymbol);
                return;
            case DeriveType::Any:
                TU_RAISE_IF_NOT_OK (definitionNode->putAttr(kLyricAstDeriveType, DeriveType::Sealed));
                return;
        }
    }

    if (ctx->finalModifier()) {
        if (isAbstract) {
            logErrorOrThrow(ctx->getStart(), "{} cannot be declared final and abstract", currentSymbol);
            return;
        }
        switch (derive) {
            case DeriveType::Final:
                logErrorOrThrow(ctx->getStart(), "{} is already declared final", currentSymbol);
                return;
            case DeriveType::Sealed:
                logErrorOrThrow(ctx->getStart(), "{} is already declared sealed", currentSymbol);
                return;
            case DeriveType::Any:
                TU_RAISE_IF_NOT_OK (definitionNode->putAttr(kLyricAstDeriveType, DeriveType::Final));
                return;
        }
    }

    TU_UNREACHABLE();
}

void
lyric_parser::internal::ModuleDefineOps::enterGlobalSpec(ModuleParser::GlobalSpecContext *ctx)
{
    auto *state = getState();
    TU_RAISE_IF_STATUS (state->peekNode(lyric_schema::kLyricAstGlobalClass));
}

void
lyric_parser::internal::ModuleDefineOps::enterGlobalVal(ModuleParser::GlobalValContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefineOps::enterGlobalVal");
}

void
lyric_parser::internal::ModuleDefineOps::exitGlobalVal(ModuleParser::GlobalValContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    // pop the top of the symbol stack and verify that the identifier matches
    auto id = ctx->symbolIdentifier()->getText();
    state->popSymbolAndCheck(id);

    auto isHidden = identifier_is_hidden(id);
    auto *typeNode = make_Type_node(state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    if (hasError())
        return;

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, state->popNode());

    ArchetypeNode *defstaticNode;
    TU_ASSIGN_OR_RAISE (defstaticNode, state->appendNode(lyric_schema::kLyricAstDefStaticClass, location));
    TU_RAISE_IF_NOT_OK (defstaticNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (defstaticNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (defstaticNode->putAttr(kLyricAstIsHidden, isHidden));
    TU_RAISE_IF_NOT_OK (defstaticNode->putAttr(kLyricAstIsVariable, false));
    TU_RAISE_IF_NOT_OK (defstaticNode->putAttr(kLyricAstTypeOffset, typeNode));
    TU_RAISE_IF_NOT_OK (state->pushNode(defstaticNode));
}

void
lyric_parser::internal::ModuleDefineOps::enterGlobalVar(ModuleParser::GlobalVarContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefineOps::enterGlobalVar");
}

void
lyric_parser::internal::ModuleDefineOps::exitGlobalVar(ModuleParser::GlobalVarContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    // pop the top of the symbol stack and verify that the identifier matches
    auto id = ctx->symbolIdentifier()->getText();
    state->popSymbolAndCheck(id);

    auto isHidden = identifier_is_hidden(id);
    auto *typeNode = make_Type_node(state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    if (hasError())
        return;

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, state->popNode());

    ArchetypeNode *defstaticNode;
    TU_ASSIGN_OR_RAISE (defstaticNode, state->appendNode(lyric_schema::kLyricAstDefStaticClass, location));
    TU_RAISE_IF_NOT_OK (defstaticNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (defstaticNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (defstaticNode->putAttr(kLyricAstIsHidden, isHidden));
    TU_RAISE_IF_NOT_OK (defstaticNode->putAttr(kLyricAstIsVariable, true));
    TU_RAISE_IF_NOT_OK (defstaticNode->putAttr(kLyricAstTypeOffset, typeNode));
    TU_RAISE_IF_NOT_OK (state->pushNode(defstaticNode));
}

void
lyric_parser::internal::ModuleDefineOps::exitGlobalSpec(ModuleParser::GlobalSpecContext *ctx)
{
    auto *state = getState();

    ArchetypeNode *elementNode;
    TU_ASSIGN_OR_RAISE (elementNode, state->popNode());

    ArchetypeNode *globalNode;
    TU_ASSIGN_OR_RAISE (globalNode, state->peekNode(lyric_schema::kLyricAstGlobalClass));

    TU_RAISE_IF_NOT_OK (globalNode->appendChild(elementNode));
}

void
lyric_parser::internal::ModuleDefineOps::parseBindingAliasStatement(ModuleParser::BindingAliasStatementContext *ctx)
{
    tempo_tracing::LeafScope scope("lyric_parser::internal::ModuleDefineOps::parseBindingAliasStatement");

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    auto *bindingAlias = ctx->bindingAlias();

    // pop the top of the symbol stack and verify that the identifier matches
    auto id = bindingAlias->symbolIdentifier()->getText();
    state->popSymbolAndCheck(id);

    auto isHidden = identifier_is_hidden(id);
    auto *typeNode = make_Type_node(state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    if (hasError())
        return;

    ArchetypeNode *aliasNode;
    TU_ASSIGN_OR_RAISE (aliasNode, state->appendNode(lyric_schema::kLyricAstAliasClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(aliasNode));

    TU_RAISE_IF_NOT_OK (aliasNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (aliasNode->putAttr(kLyricAstIsHidden, isHidden));
    TU_RAISE_IF_NOT_OK (aliasNode->putAttr(kLyricAstTypeOffset, typeNode));

    // generic information
    if (bindingAlias->placeholderSpec()) {
        auto *placeholderSpec = bindingAlias->placeholderSpec();
        auto *genericNode = make_Generic_node(state, placeholderSpec);
        TU_RAISE_IF_NOT_OK (aliasNode->putAttr(kLyricAstGenericOffset, genericNode));
    }
}

void
lyric_parser::internal::ModuleDefineOps::parseIndexAliasStatement(ModuleParser::IndexAliasStatementContext *ctx)
{
    tempo_tracing::LeafScope scope("lyric_parser::internal::ModuleDefineOps::parseIndexAliasStatement");

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    auto *indexAlias = ctx->indexAlias();

    // pop the top of the symbol stack and verify that the identifier matches
    auto id = indexAlias->symbolIdentifier()->getText();
    state->popSymbolAndCheck(id);

    auto isHidden = identifier_is_hidden(id);
    auto *typeNode = make_Type_node(state, ctx->assignableType());

    std::vector<std::string> parts;
    for (size_t i = 0; i < indexAlias->symbolPath()->getRuleIndex(); i++) {
        if (indexAlias->symbolPath()->Identifier(i) == nullptr)
            continue;
        parts.push_back(indexAlias->symbolPath()->Identifier(i)->getText());
    }
    lyric_common::SymbolPath symbolPath(parts);

    auto indexLiteral = indexAlias->DecimalInteger()->getText();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    if (hasError())
        return;

    ArchetypeNode *aliasNode;
    TU_ASSIGN_OR_RAISE (aliasNode, state->appendNode(lyric_schema::kLyricAstAliasClass, location));

    TU_RAISE_IF_NOT_OK (aliasNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (aliasNode->putAttr(kLyricAstIsHidden, isHidden));
    TU_RAISE_IF_NOT_OK (aliasNode->putAttr(kLyricAstLiteralValue, indexLiteral));
    TU_RAISE_IF_NOT_OK (aliasNode->putAttr(kLyricAstSymbolPath, symbolPath));
    TU_RAISE_IF_NOT_OK (aliasNode->putAttr(kLyricAstTypeOffset, typeNode));

    TU_RAISE_IF_NOT_OK (state->pushNode(aliasNode));
}

void
lyric_parser::internal::ModuleDefineOps::parseKeyAliasStatement(ModuleParser::KeyAliasStatementContext *ctx)
{
    tempo_tracing::LeafScope scope("lyric_parser::internal::ModuleDefineOps::parseIndexAliasStatement");

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    auto *keyAlias = ctx->keyAlias();

    // pop the top of the symbol stack and verify that the identifier matches
    auto id = keyAlias->symbolIdentifier()->getText();
    state->popSymbolAndCheck(id);

    auto isHidden = identifier_is_hidden(id);
    auto *typeNode = make_Type_node(state, ctx->assignableType());

    std::vector<std::string> parts;
    for (size_t i = 0; i < keyAlias->symbolPath()->getRuleIndex(); i++) {
        if (keyAlias->symbolPath()->Identifier(i) == nullptr)
            continue;
        parts.push_back(keyAlias->symbolPath()->Identifier(i)->getText());
    }
    lyric_common::SymbolPath symbolPath(parts);

    auto keyLiteral = keyAlias->Identifier()->getText();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    if (hasError())
        return;

    ArchetypeNode *aliasNode;
    TU_ASSIGN_OR_RAISE (aliasNode, state->appendNode(lyric_schema::kLyricAstAliasClass, location));

    TU_RAISE_IF_NOT_OK (aliasNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (aliasNode->putAttr(kLyricAstIsHidden, isHidden));
    TU_RAISE_IF_NOT_OK (aliasNode->putAttr(kLyricAstLiteralValue, keyLiteral));
    TU_RAISE_IF_NOT_OK (aliasNode->putAttr(kLyricAstSymbolPath, symbolPath));
    TU_RAISE_IF_NOT_OK (aliasNode->putAttr(kLyricAstTypeOffset, typeNode));

    TU_RAISE_IF_NOT_OK (state->pushNode(aliasNode));
}
